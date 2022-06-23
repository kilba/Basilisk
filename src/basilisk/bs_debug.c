#include <bs_core.h>
#include <stdio.h>
#include <cglm/cglm.h>

// TODO:
// Städa
// Increase/decrease speed
// Deltatime

float yaw   = -90.0f;
float pitch =  0.0f;
float lastX =  1200.0 / 2.0;
float lastY =  800.0 / 2.0;

bool freecam_mode = false;
float cam_speed = 1.0;

bs_Camera cam;
vec3 cameraFront = (vec3){ 0.0f, 0.0f, -1.0f };

bs_Camera pre_freecam_cam;

bs_vec3 bs_getCamRotation() {
	bs_vec2 pos = bs_getCursorPositionReverseY();

    float xoffset = pos.x - lastX;
    float yoffset = lastY - pos.y; 
    lastX = pos.x;
    lastY = pos.y;

    float sensitivity = 0.15;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f) {
        pitch = 89.0f;
    }
    if(pitch < -89.0f) {
    	pitch = -89.0f;
    }

    bs_vec3 direction;
    direction.x = cos(glm_rad(yaw)) * cos(glm_rad(pitch));
    direction.y = sin(glm_rad(pitch));
    direction.z = sin(glm_rad(yaw)) * cos(glm_rad(pitch));
    glm_normalize((vec3){ direction.x, direction.y, direction.z });
    return direction;
}

vec3 cam_pos = { 0.0, 0.0, 0.0 };
void bs_setCamMovement(bs_Camera *cam, bs_vec3 dir) {
	float cam_speed = 1.0;
	vec3 cam_front = { dir.x, dir.y, dir.z };
	vec3 add = { 0.0, 0.0, 0.0 };
	vec3 up = { 0.0, 1.0, 0.0 };

	if(bs_isKeyDown(BS_KEY_D)) {
		vec3 cross;
		glm_cross(cam_front, up, cross);
		glm_normalize(cross);

		glm_vec3_muladds(cross, cam_speed, add);
	}
	if(bs_isKeyDown(BS_KEY_A)) {
		vec3 cross;
		glm_cross(cam_front, up, cross);
		glm_normalize(cross);

		glm_vec3_muladds(cross, -cam_speed, add);
	}
	if(bs_isKeyDown(BS_KEY_W)) {
		glm_vec3_muladds(cam_front, cam_speed, add);
	}
	if(bs_isKeyDown(BS_KEY_S)) {
		glm_vec3_muladds(cam_front, -cam_speed, add);
	}

	cam->pos.x += add[0];
	cam->pos.y += add[1];
	cam->pos.z += add[2];
}

void bs_enterFreecam(bs_Camera *cam) {
	bs_vec3 dir = bs_getCamRotation();

    // bs_setMatrixLookat(&cam, (bs_vec3){ 0.0, 0.0, 2.0 }, (bs_vec3){ 0.0, 1.0, 0.0 });
    // bs_setPerspectiveProjection(&cam, (bs_vec2){ 1200.0, 800.0 }, 90.0, 0.1, 5000.0);

	bs_setCamMovement(cam, dir);
    bs_setMatrixLookat(cam, (bs_vec3){ cam->pos.x + dir.x, cam->pos.y + dir.y, cam->pos.z + dir.z }, (bs_vec3){ 0.0, 1.0, 0.0 });
    bs_setPerspectiveProjection(cam, cam->res, 90.0, 0.1, 5000.0);
}

void bs_debugUpdate() {
	if(bs_isKeyDownOnce(BS_KEY_O)) {
		if(!freecam_mode) {
			pre_freecam_cam = *bs_getStdCamera();
		} else {
			*bs_getStdCamera() = pre_freecam_cam;
		}
		freecam_mode = !freecam_mode;
	}

	if(freecam_mode) {
		bs_Camera *cam = bs_getStdCamera();
		bs_enterFreecam(cam);
	}
}

void bs_debugStart() {
    bs_pushBatch();
    bs_freeBatchData();
}