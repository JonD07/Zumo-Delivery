#include "Filter.h"

void SanityPrint(Filter_Data_t* p_filt) {
	int n = rb_length_F(&p_filt->in_list);

	printf("in: ");
	for(int i = 0; i < n; i++) {
		printf("%f ", rb_get_F(&p_filt->in_list, i));
	}

	printf("\nout: ");
	for(int i = 0; i < n; i++) {
		printf("%f ", rb_get_F(&p_filt->out_list, i));
	}
	puts("");
}

/**
 * Function Filter_Init initializes the filter given two float arrays and the order of the filter.  Note that the
 * size of the array will be one larger than the order. (First order systems have two coefficients).
 *
 *  1/A_0*( SUM( B_i * input_i )  -   SUM( A_i * output_i) )
 *         i=0..N                    i=1..N
 *
 *  Note a 5-point moving average filter has coefficients:
 *      numerator_coeffs   = { 5 0 0 0 0 };
 *      denominator_coeffs = { 1 1 1 1 1 };
 *      order = 4;
 *
 * @param p_filt pointer to the filter object
 * @param numerator_coeffs The numerator coefficients (B/beta traditionally)
 * @param denominator_coeffs The denominator coefficients (A/alpha traditionally)
 * @param order The filter order
 */
void  Filter_Init ( Filter_Data_t* p_filt, float* numerator_coeffs, float* denominator_coeffs, uint8_t order ) {
	// Initialize buffers
	rb_initialize_F(&p_filt->numerator);
	rb_initialize_F(&p_filt->denominator);
	rb_initialize_F(&p_filt->in_list);
	rb_initialize_F(&p_filt->out_list);

	// Store alpha/beta values and initial input/outputs
	for(int i = 0; i <= order; i++) {
		rb_push_back_F(&p_filt->numerator, numerator_coeffs[i]);
		rb_push_back_F(&p_filt->denominator, denominator_coeffs[i]);
		rb_push_back_F(&p_filt->in_list, 0);
		rb_push_back_F(&p_filt->out_list, 0);
	}

	return;
}

/**
 * Function Filter_ShiftBy shifts the input list and output list to keep the filter in the same frame. This is especially
 * useful when initializing the filter to the current value or handling wrapping/overflow issues.
 * @param p_filt
 * @param shift_amount
 */
void  Filter_ShiftBy( Filter_Data_t* p_filt, float shift_amount ) {
	int n = rb_length_F(&p_filt->in_list);
	for(int i = 0; i < n; i++) {
		rb_set_F(&p_filt->in_list, i, rb_get_F(&p_filt->in_list, i) + shift_amount);
		rb_set_F(&p_filt->out_list, i, rb_get_F(&p_filt->out_list, i) + shift_amount);
	}
}

/**
 * Function Filter_SetTo sets the initial values for the input and output lists to a constant defined value. This
 * helps to initialize or re-initialize the filter as desired.
 * @param p_filt Pointer to a Filter_Data sturcture
 * @param amount The value to re-initialize the filter to.
 */
void Filter_SetTo( Filter_Data_t* p_filt, float amount ) {
	int n = rb_length_F(&p_filt->in_list);

	for(int i = 0; i < n; i++) {
		rb_set_F(&p_filt->in_list, i, amount);
		rb_set_F(&p_filt->out_list, i, amount);
	}
}

/**
 * Function Filter_Value adds a new value to the filter and returns the new output.
 * @param p_filt pointer to the filter object
 * @param value of the new measurement
 * @return The newly filtered value
 *
 *  1/A_0*( SUM( B_i * input_i )  -   SUM( A_i * output_i) )
 *         i=0..N                    i=1..N
 *
 */
float Filter_Value( Filter_Data_t* p_filt, float value) {
	float top_sum = 0, bottum_sum = 0;
	int n = rb_length_F(&p_filt->in_list);

	// Pop oldest input/output value
	rb_pop_back_F(&p_filt->in_list);
	rb_pop_back_F(&p_filt->out_list);
	// Push newest reading
	rb_push_front_F(&p_filt->in_list, value);

	for(int i = 0; i < n; i++) {
		top_sum += rb_get_F(&p_filt->numerator, i) * rb_get_F(&p_filt->in_list, i);
		if(i < (n - 1)) {
			bottum_sum += rb_get_F(&p_filt->denominator, i + 1) * rb_get_F(&p_filt->out_list, i);
		}
	}

	// Push new output onto buffer
	rb_push_front_F(&p_filt->out_list, (top_sum - bottum_sum) / rb_get_F(&p_filt->denominator, 0));

	return rb_get_F(&p_filt->out_list, 0);
}

/**
 * Function Filter_Last_Output returns the most up-to-date filtered value without updating the filter.
 * @return The latest filtered value
 */
float Filter_Last_Output( Filter_Data_t* p_filt ) {
	return rb_get_F(&p_filt->out_list, 0);
}
