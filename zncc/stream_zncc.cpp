#include "hls_stream.h"
#include "ap_uint.h" 

struct data_struct{
  ap_uint<96> data;
  bool last;
};

// ============= ARCHITECTURE PARAMETERS =============
// Define the number of points of the correlation function computed each iteration,
// the size of local buffers
int const LOCAL_BUFF_DIM = 80;
// Define the partition factor of the local buffers.
int const PARTITION_FACTOR = 20;



void corr2d( // ============= PARAMETERS =============
        hls::stream<float> &stream_in,             // Input stream on which the signals are passed.
        hls::stream<data_struct> &stream_out     // Output stream, on which the correlation function is written.
        ) {
#pragma HLS INTERFACE axis port=stream_in
#pragma HLS INTERFACE axis port=stream_out
#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS ALLOCATION instances=fmul limit=MAX_PARALLEL_FMUL operation

    // Data struct to save results
    data_struct out_data;

    // ============= READ PARAMETERS FROM STREAMS =============
    // To avoid add more than two input data access the input parameters are pushed at the beginning of the streams.
    float f_n_iterations = stream_in.read(); // Number of points in the original target signal.

    int n_iterations = (int) f_n_iterations;

    float buff_sum[LOCAL_BUFF_DIM];
    float buf_pow[LOCAL_BUFF_DIM];

    // ============= LOCAL VARIABLES =============
    float temp_in;


    float local_out_buffer[LOCAL_BUFF_DIM];
#pragma HLS ARRAY_PARTITION variable=local_out_buffer cyclic factor=PARTITION_FACTOR dim=0


    float local_template_buffer[LOCAL_BUFF_DIM];
#pragma HLS ARRAY_PARTITION variable=local_template_buffer cyclic factor=PARTITION_FACTOR dim=0

    // ============= OUT BUFFER INIT =============
    init_local_out_buffer: for (int j = 0; j < LOCAL_BUFF_DIM; j++) {
#pragma HLS UNROLL
        local_out_buffer[j] = 0.0;
        buff_sum[j]= 0.0;
        buf_pow[j]= 0.0;
    }

    // ============= TEMPLATE BUFFER INIT =============
    init_local_template_buffer: for(int j = 0; j < LOCAL_BUFF_DIM; j++) {
#pragma HLS PIPELINE II=1
        local_template_buffer[j] = stream_in.read();
    }


    // ============= CORRELATION COMPUTATION =============
    iterations: for (int i = 0; i < n_iterations; i++) {
#pragma HLS LOOP_TRIPCOUNT min=10 max=10 avg=10
#pragma HLS PIPELINE II=1

        temp_in = stream_in.read();

        parallel_mul: for(int j = 0; j < LOCAL_BUFF_DIM; j++){
#pragma HLS UNROLL
            local_out_buffer[j] += temp_in * local_template_buffer[j];
            buff_sum[j]+=temp_in;
            buf_pow[j]+=temp_in*temp_in;
        }


        apu_uint<96> out;
        out.range(31,0)= *((float*)&local_out_buffer[LOCAL_BUFF_DIM-1]);
        out.range(63,32)= *((float*)&buff_sum[LOCAL_BUFF_DIM-1]);
        out.range(95,64)= *((float*)&buf_pow[LOCAL_BUFF_DIM-1]);

        out_data.data = out;
        out_data.last = (i == n_iterations - 1) ? 1 : 0;

        // Shift by one
        shift: for(int k = LOCAL_BUFF_DIM - 1; k > 0; --k) {
#pragma HLS UNROLL
            local_out_buffer[k] = local_out_buffer[k - 1];
            buff_sum[k] = buff_sum[k-1];
            buf_pow[k] = buf_pow[k-1];
        }
        local_out_buffer[0] = 0.0;
        buff_sum[0]=0.0;
        buf_pow[0]=0.0;

        stream_out.write(out_data);
    }
}
