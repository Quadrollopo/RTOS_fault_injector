#ifndef RTOSDEMO_CONFIG_H
#define RTOSDEMO_CONFIG_H

#define PARALLELIZATION 0
#define WRITE_ON_FILE 1
#if PARALLELIZATION
#define HW_CONCURRENCY_FACTOR 2
#endif

#endif //RTOSDEMO_CONFIG_H
