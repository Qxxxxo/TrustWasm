/**
 * @brief test async poll overhead
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include "2mm.h"
#include "polybench.h"

#define SUCCESS 0
#define SHORT_BUFFER 0xFFFF0010

__attribute__((import_name("dht_init"))) int dht_init(uint32_t pin, uint32_t type);

__attribute__((import_name("dht_read"))) int
dht_read(uint32_t pin, uint32_t type, uint32_t dst_temp_addr, uint32_t dst_hum_addr);

__attribute__((import_name("dht_read_async"))) int dht_read_async(uint32_t pin, uint32_t type, void (*handler)(uint32_t, uint32_t));

__attribute__((import_name("native_wait"))) int native_wait(int ms);

__attribute__((import_name("get_async_data"))) int get_async_data(uint32_t signo,
                                                                  uint32_t sub_signo,
                                                                  uint32_t dst_buf_addr,
                                                                  uint32_t dst_buf_len_addr);
__attribute__((import_name("record_benchmark_time"))) int record_benchmark_time(uint32_t idx);
__attribute__((import_name("get_benchmark_time"))) int get_benchmark_time(uint32_t idx,uint32_t out_time_addr);

void handler(uint32_t signo, uint32_t sub_signo)
{
    int16_t *buf = (int16_t *)malloc(2 * sizeof(int16_t));
    uint32_t len = 2 * sizeof(int16_t);
    int ret = get_async_data(signo, sub_signo, (uint32_t)buf, (uint32_t)&len);
    record_benchmark_time(5);
    // if (ret == SHORT_BUFFER)
    // {
    //     // need buf for for suggest len
    //     int16_t *new_buf = (int16_t *)realloc(buf, len);
    //     // try again
    //     ret = get_async_data(signo, sub_signo, (uint32_t)new_buf, (uint32_t)&len);
    //     if (ret > 0)
    //     {
    //         printf("signo %d, sub_signo %d success: temperature %d.%d°C, humdity %d.%d%%\n",
    //                signo,sub_signo,new_buf[0] / 10, new_buf[0] % 10, new_buf[1] / 10, new_buf[1] % 10);
    //     }
    // }
    // else if (ret > 0)
    // {
    //     printf("signo %d, sub_signo %d success: temperature %d.%d°C, humdity %d.%d%%\n",
    //            signo,sub_signo,buf[0] / 10, buf[0] % 10, buf[1] / 10, buf[1] % 10);
    // }else{
    //     printf("signo %d, sub_signo %d failed\n",signo,sub_signo);
    // }
}

/* Array initialization. */
static
void init_array(int ni, int nj, int nk, int nl,
		DATA_TYPE *alpha,
		DATA_TYPE *beta,
		DATA_TYPE POLYBENCH_2D(A,NI,NK,ni,nk),
		DATA_TYPE POLYBENCH_2D(B,NK,NJ,nk,nj),
		DATA_TYPE POLYBENCH_2D(C,NJ,NL,nj,nl),
		DATA_TYPE POLYBENCH_2D(D,NI,NL,ni,nl))
{
  int i, j;

  *alpha = 1.5;
  *beta = 1.2;
  for (i = 0; i < ni; i++)
    for (j = 0; j < nk; j++)
      A[i][j] = (DATA_TYPE) ((i*j+1) % ni) / ni;
  for (i = 0; i < nk; i++)
    for (j = 0; j < nj; j++)
      B[i][j] = (DATA_TYPE) (i*(j+1) % nj) / nj;
  for (i = 0; i < nj; i++)
    for (j = 0; j < nl; j++)
      C[i][j] = (DATA_TYPE) ((i*(j+3)+1) % nl) / nl;
  for (i = 0; i < ni; i++)
    for (j = 0; j < nl; j++)
      D[i][j] = (DATA_TYPE) (i*(j+2) % nk) / nk;
}

/* Main computational kernel. The whole function will be timed,
   including the call and return. */
static
void kernel_2mm(int ni, int nj, int nk, int nl,
		DATA_TYPE alpha,
		DATA_TYPE beta,
		DATA_TYPE POLYBENCH_2D(tmp,NI,NJ,ni,nj),
		DATA_TYPE POLYBENCH_2D(A,NI,NK,ni,nk),
		DATA_TYPE POLYBENCH_2D(B,NK,NJ,nk,nj),
		DATA_TYPE POLYBENCH_2D(C,NJ,NL,nj,nl),
		DATA_TYPE POLYBENCH_2D(D,NI,NL,ni,nl))
{
  int i, j, k;

#pragma scop
  /* D := alpha*A*B*C + beta*D */
  for (i = 0; i < _PB_NI; i++)
    for (j = 0; j < _PB_NJ; j++)
      {
	tmp[i][j] = SCALAR_VAL(0.0);
	for (k = 0; k < _PB_NK; ++k)
	  tmp[i][j] += alpha * A[i][k] * B[k][j];
      }
  for (i = 0; i < _PB_NI; i++)
    for (j = 0; j < _PB_NL; j++)
      {
	D[i][j] *= beta;
	for (k = 0; k < _PB_NJ; ++k)
	  D[i][j] += tmp[i][k] * C[k][j];
      }
#pragma endscop

}

/* DCE code. Must scan the entire live-out data.
   Can be used also to check the correctness of the output. */
static
void print_array(int ni, int nl,
		 DATA_TYPE POLYBENCH_2D(D,NI,NL,ni,nl))
{
  int i, j;

  POLYBENCH_DUMP_START;
  POLYBENCH_DUMP_BEGIN("D");
  for (i = 0; i < ni; i++)
    for (j = 0; j < nl; j++) {
	if ((i * ni + j) % 20 == 0) fprintf (POLYBENCH_DUMP_TARGET, "\n");
	fprintf (POLYBENCH_DUMP_TARGET, DATA_PRINTF_MODIFIER, D[i][j]);
    }
  POLYBENCH_DUMP_END("D");
  POLYBENCH_DUMP_FINISH;
}

int run_2mm(int argc,char ** argv)
{
      /* Retrieve problem size. */
  int ni = NI;
  int nj = NJ;
  int nk = NK;
  int nl = NL;

  /* Variable declaration/allocation. */
  DATA_TYPE alpha;
  DATA_TYPE beta;
  POLYBENCH_2D_ARRAY_DECL(tmp,DATA_TYPE,NI,NJ,ni,nj);
  POLYBENCH_2D_ARRAY_DECL(A,DATA_TYPE,NI,NK,ni,nk);
  POLYBENCH_2D_ARRAY_DECL(B,DATA_TYPE,NK,NJ,nk,nj);
  POLYBENCH_2D_ARRAY_DECL(C,DATA_TYPE,NJ,NL,nj,nl);
  POLYBENCH_2D_ARRAY_DECL(D,DATA_TYPE,NI,NL,ni,nl);

  /* Initialize array(s). */
  init_array (ni, nj, nk, nl, &alpha, &beta,
	      POLYBENCH_ARRAY(A),
	      POLYBENCH_ARRAY(B),
	      POLYBENCH_ARRAY(C),
	      POLYBENCH_ARRAY(D));

  /* Start timer. */
  polybench_start_instruments;

  /* Run kernel. */
  kernel_2mm (ni, nj, nk, nl,
	      alpha, beta,
	      POLYBENCH_ARRAY(tmp),
	      POLYBENCH_ARRAY(A),
	      POLYBENCH_ARRAY(B),
	      POLYBENCH_ARRAY(C),
	      POLYBENCH_ARRAY(D));

  /* Stop and print timer. */
  polybench_stop_instruments;
  polybench_print_instruments;

  /* Prevent dead-code elimination. All live-out data must be printed
     by the function call in argument. */
  polybench_prevent_dce(print_array(ni, nl,  POLYBENCH_ARRAY(D)));

  /* Be clean. */
  POLYBENCH_FREE_ARRAY(tmp);
  POLYBENCH_FREE_ARRAY(A);
  POLYBENCH_FREE_ARRAY(B);
  POLYBENCH_FREE_ARRAY(C);
  POLYBENCH_FREE_ARRAY(D);

  return 0;
}

int main(int argc, char **argv)
{
    uint64_t start_time,end_time;
    int16_t temp = 0;
    int16_t hum = 0;
    record_benchmark_time(0);
    run_2mm(argc,argv);
    record_benchmark_time(1);
    get_benchmark_time(0,(uint32_t)&start_time);
    get_benchmark_time(1,(uint32_t)&end_time);
    printf("w/o poll start time: %llu, end time: %llu, interval: %llu\n",start_time,end_time,end_time-start_time);
    // init as DHT11, pin 22
    if (dht_init(22, 0) == SUCCESS)
    {   
        record_benchmark_time(0);
        dht_read(22, 0, (uint32_t)&temp, (uint32_t)&hum);
        record_benchmark_time(1);

        record_benchmark_time(4);
        if (dht_read_async(22, 0, handler) == SUCCESS)
        {
            // printf("async read send\n");
            record_benchmark_time(2);
            run_2mm(argc,argv);
            record_benchmark_time(3);
        }
    }
    get_benchmark_time(0,(uint32_t)&start_time);
    get_benchmark_time(1,(uint32_t)&end_time);
    printf("dht sync read start time: %llu, end time: %llu, interval: %llu\n",start_time,end_time,end_time-start_time);
    get_benchmark_time(2,(uint32_t)&start_time);
    get_benchmark_time(3,(uint32_t)&end_time);
    printf("w/ poll run 2mm start time: %llu, end time: %llu, interval: %llu\n",start_time,end_time,end_time-start_time);
    get_benchmark_time(4,(uint32_t)&start_time);
    get_benchmark_time(5,(uint32_t)&end_time);
    printf("dht aync read handle start time: %llu, end time: %llu, interval: %llu\n",start_time,end_time,end_time-start_time);
    return 0;
}
