#include <CL/cl.h>
#include <stdio.h>

#define MAX_SOURCE_SIZE (0x100000)
#define ARRAY_SIZE 10

int main() {

    // Загрузка кода ядра из файла
    FILE *kernelFile = fopen("test.cl", "r");
    if (!kernelFile) {
        fprintf(stderr, "Не удалось открыть файл с кодом ядра.\n");
        return 1;
    }

    char *kernelSource = (char *)malloc(MAX_SOURCE_SIZE);
    size_t kernelSize = fread(kernelSource, 1, MAX_SOURCE_SIZE, kernelFile);
    fclose(kernelFile);

    // Получение доступных платформ
    cl_uint numPlatforms;
    clGetPlatformIDs(0, NULL, &numPlatforms);

    cl_platform_id *platforms = (cl_platform_id *)malloc(numPlatforms * sizeof(cl_platform_id));
    clGetPlatformIDs(numPlatforms, platforms, NULL);

    // Выбор первой платформы
    cl_platform_id platform = platforms[0];

    // Получение доступных устройств
    cl_uint numDevices;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices);

    cl_device_id *devices = (cl_device_id *)malloc(numDevices * sizeof(cl_device_id));
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, numDevices, devices, NULL);

    // Выбор первого устройства
    cl_device_id device = devices[0];

    // Создание контекста
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);

    // Создание командной очереди
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, NULL);

    int count = 10;
    int width = 3;
    int* comb = (int*) malloc (sizeof(int) * count * width);
    comb[0 * width + 1] = 2;
    comb[0 * width + 0] = 2; 
    comb[0 * width + 2] = 2;

    comb[1 * width + 0] = 2;
    comb[1 * width + 1] = 2;
    comb[1 * width + 2] = 3;

    comb[2 * width + 0] = 2;
    comb[2 * width + 1] = 2;
    comb[2 * width + 2] = 5;

    comb[3 * width + 0] = 2; 
    comb[3 * width + 1] = 3;
    comb[3 * width + 2] = 2;

    comb[4 * width + 0] = 2;
    comb[4 * width + 1] = 3;
    comb[4 * width + 2] = 3;

    comb[5 * width + 0] = 2;
    comb[5 * width + 1] = 3;
    comb[5 * width + 2] = 5;

    comb[6 * width + 0] = 2; 
    comb[6 * width + 1] = 5;
    comb[6 * width + 2] = 2;

    comb[7 * width + 0] = 2;
    comb[7 * width + 1] = 5;
    comb[7 * width + 2] = 3;

    comb[8 * width + 0] = 2;
    comb[8 * width + 1] = 5;
    comb[8 * width + 2] = 5;

    comb[9 * width + 0] = 3;
    comb[9 * width + 1] = 2;
    comb[9 * width + 2] = 2;

    cl_mem buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int) * count * width, comb, NULL);

    // Загрузка программы в контекст
    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&kernelSource, &kernelSize, NULL);
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);

    // Создание ядра
    cl_kernel kernel = clCreateKernel(program, "find_max", NULL);

    // Установка параметров ядра
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer);
    clSetKernelArg(kernel, 1, sizeof(int), &count);

    // Выполнение программы на устройстве
    size_t globalWorkSize = count * width;
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalWorkSize, NULL, 0, NULL, NULL);

    // Получение результатов
    int *resultArray = (int *)malloc(sizeof(int) * count * width);
    clEnqueueReadBuffer(queue, buffer, CL_TRUE, 0, sizeof(int)  * count * width, resultArray, 0, NULL, NULL);

    printf("Original array: \n");
    for (int i = 0; i < count; i++) {
        printf("%d %d %d\n", comb[i * width + 0], comb[i * width + 1], comb[i * width + 2]);
    }

    printf("\nResult array: \n");
    for (int i = 0; i < count; i++) {
        printf("%d\n", resultArray[i * width]);
    }

    // Освобождение ресурсов
    clReleaseMemObject(buffer);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    free(kernelSource);
    free(platforms);
    free(devices);
    free(resultArray);

    return 0;
}