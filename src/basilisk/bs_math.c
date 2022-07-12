int bs_sign(float x) {
	return (x > 0) - (x < 0);
}

double bs_fMap(double input, double input_start, double input_end, double output_start, double output_end) {
	double slope = 1.0 * (output_end - output_start) / (input_end - input_start);
	return output_start + slope * (input - input_start);
}