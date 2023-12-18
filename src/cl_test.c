#include <CL/cl.h>
#include <stdio.h>

#define MAX_SOURCE_SIZE (0x100000)
#define ARRAY_SIZE 10

int main() {
    // Загрузка кода ядра из файла
    FILE *kernelFile = fopen("square.cl", "r");
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
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &numDevices);

    cl_device_id *devices = (cl_device_id *)malloc(numDevices * sizeof(cl_device_id));
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, numDevices, devices, NULL);

    // Выбор первого устройства
    cl_device_id device = devices[0];

    // Создание контекста
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);

    // Создание командной очереди
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, NULL);

    // Создание буфера для массива данных
    int inputArray[ARRAY_SIZE] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    cl_mem buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int) * ARRAY_SIZE, inputArray, NULL);

    // Создание буфера для максимального значения
    int globalMaxValue = 0; // Изначально устанавливаем в 0
    cl_mem globalMaxBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int), &globalMaxValue, NULL);

    // Загрузка программы в контекст
    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&kernelSource, &kernelSize, NULL);
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);

    // Создание ядра
    cl_kernel kernel = clCreateKernel(program, "square", NULL);

    // Установка параметров ядра
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &globalMaxBuffer);

    // Выполнение программы на устройстве
    size_t globalWorkSize = ARRAY_SIZE;
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalWorkSize, NULL, 0, NULL, NULL);

    // Получение результатов
    int *resultArray = (int *)malloc(sizeof(int) * ARRAY_SIZE);
    clEnqueueReadBuffer(queue, buffer, CL_TRUE, 0, sizeof(int) * ARRAY_SIZE, resultArray, 0, NULL, NULL);
    clEnqueueReadBuffer(queue, globalMaxBuffer, CL_TRUE, 0, sizeof(int), &globalMaxValue, 0, NULL, NULL);

    // Вывод глобального максимального значения
    printf("\nGlobal Max: %d\n", globalMaxValue);

    // Вывод результатов
    printf("Original array: ");
    for (int i = 0; i < ARRAY_SIZE; i++) {
        printf("%d ", inputArray[i]);
    }

    printf("\nSquared array: ");
    for (int i = 0; i < ARRAY_SIZE; i++) {
        printf("%d ", resultArray[i]);
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