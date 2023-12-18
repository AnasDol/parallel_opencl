#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <omp.h>
#include <CL/cl.h>

typedef struct {
    int max;
    int a, b, c;
} Result;

Result find_max(int* primes, int count, int N) {
    
    Result result;
    result.max = -1;
    result.a = -1;
    result.b = -1;
    result.c = -1;

    if (N < 28) return result;

    for (int i = 0; i < count; i++) {
        for (int j = 0; j < count; j++) {
            for (int k = 0; k < count; k++) {
                int max = primes[i]*primes[i] + primes[j]*primes[j]*primes[j] + primes[k]*primes[k]*primes[k]*primes[k];
                if (max <= N && max > result.max) {
                    result.max = max;
                    result.a = primes[i];
                    result.b = primes[j];
                    result.c = primes[k];
                }
            }
        }
    }
    return result;
}

void check_error(cl_int error, const char* message) {
    if (error != CL_SUCCESS) {
        fprintf(stderr, "OpenCL Error: %s (error code %d)\n", message, error);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]) {

    int N;
    printf("N = ", N);
    if (!(scanf("%d", &N)==1 && N>=1)) {
        printf("Input error\n");
        return 0;
    }

    if (N < 28) {
        printf("Too small number. MIN = 28.");
        return 0;
    }

    
    // OpenCL код
    const char* program_source = "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n"
                                "__kernel void sieve(__global int* numbers, int N) {\n"
                                "    int gid = get_global_id(0);\n"
                                "    if (gid >= 2 && gid * gid <= N && numbers[gid] == 1) {\n"
                                "        for (int j = gid * gid; j <= N; j += gid) {\n"
                                "            if (numbers[j] == 1) {\n"
                                "                numbers[j] = 0;\n"
                                "            }\n"
                                "        }\n"
                                "    }\n"
                                "}\n";

    int* numbers = (int*)malloc(sizeof(int) * (N + 1));
    
    for (int i = 0; i <= N; ++i) {
        numbers[i] = 1;
    }

    // Получение доступных платформ
    cl_uint numPlatforms;
    clGetPlatformIDs(0, NULL, &numPlatforms);

    cl_platform_id *platforms = (cl_platform_id *)malloc(numPlatforms * sizeof(cl_platform_id));
    clGetPlatformIDs(numPlatforms, platforms, NULL);

    // Выбор первой платформы
    cl_platform_id platform = platforms[0];

    // Получение доступных устройств
    cl_uint numDevices;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &numDevices);

    cl_device_id *devices = (cl_device_id *)malloc(numDevices * sizeof(cl_device_id));
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, numDevices, devices, NULL);

    // Выбор первого устройства
    cl_device_id device = devices[0];

    // Создание контекста
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);

    // Создание командной очереди
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, NULL);



    cl_mem numbers_buffer;
    
    

    cl_platform_id platform;
    clGetPlatformIDs(1, &platform, NULL);

    cl_device_id device;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);

    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, NULL);
    
    numbers_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int) * (N + 1), numbers, NULL);

    cl_program program = clCreateProgramWithSource(context, 1, &program_source, NULL, NULL);
    check_error(clBuildProgram(program, 1, &device, NULL, NULL, NULL), "Failed to build program");

    cl_kernel kernel = clCreateKernel(program, "sieve", NULL);
    check_error(clSetKernelArg(kernel, 0, sizeof(cl_mem), &numbers_buffer), "Failed to set kernel arguments");

    size_t global_work_size = N - 1;
    check_error(clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_work_size, NULL, 0, NULL, NULL), "Failed to enqueue kernel");

    check_error(clFinish(queue), "Failed to finish execution");

    clEnqueueReadBuffer(queue, numbers_buffer, CL_TRUE, 0, sizeof(int) * (N + 1), numbers, 0, NULL, NULL);

    int count = 0;
    int* primes = (int*)malloc(sizeof(int) * (N + 1));

    for (int i = 2; i <= N; ++i) {
        if (numbers[i]) {
            primes[count++] = i;
        }
    }

    printf("count: %d\n", count);

    Result result = find_max(primes, count, N);

    printf("\n---------------Output----------------\n");
    printf("%d^2 + %d^3 + %d^4 = %d\n", result.a, result.b, result.c, result.max);

    // Освобождение ресурсов OpenCL
    clReleaseMemObject(numbers_buffer);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    free(numbers);
    free(primes);

    return 0;
}