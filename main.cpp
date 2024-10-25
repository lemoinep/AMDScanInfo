#include <iostream>
#include <vector>
#include <iomanip>
#include <cstring>
#include <rocm_smi/rocm_smi.h>


// lok this web page for more informations  https://manpages.ubuntu.com/manpages/lunar/man1/rocm-smi.1.html

void checkReturn(rsmi_status_t ret, const char* message) {
    if (ret != RSMI_STATUS_SUCCESS) {
        std::cerr << message << " failed with error: " << ret << std::endl;
        exit(1);
    }
}

void writeGPUInfo(uint32_t deviceIndex) {
    rsmi_status_t ret;

    uint64_t id;
    ret = rsmi_dev_unique_id_get(deviceIndex, &id);
    checkReturn(ret, "Get device unique ID");
    std::cout << "GPU Unique ID: " << id << std::endl;

    // Display GPU name
    char name[256];
    ret = rsmi_dev_name_get(deviceIndex, name, sizeof(name));
    checkReturn(ret, "Get device name");
    std::cout << "GPU Name: " << name << std::endl;

    // Display VBIOS version
    char vbios[256];
    ret = rsmi_dev_vbios_version_get(deviceIndex, vbios, sizeof(vbios));
    checkReturn(ret, "Get VBIOS version");
    std::cout << "VBIOS version: " << vbios << std::endl;


    // Show temperature
    int64_t temp;
    ret = rsmi_dev_temp_metric_get(deviceIndex, 0, RSMI_TEMP_CURRENT, &temp);
    checkReturn(ret, "Get temperature");
    std::cout << "Temperature: " << temp/1000.0 << " °C" << std::endl;



    // Display speed ventilateur
    if (1==0) {
        int64_t speed;
        ret = rsmi_dev_fan_speed_get(deviceIndex, 0, &speed);
        checkReturn(ret, "Get fan speed");
        std::cout << "Fan speed: " << speed << " RPM" << std::endl;
    }


    // Display GPU frequency
    rsmi_frequencies_t freqs;
    ret = rsmi_dev_gpu_clk_freq_get(deviceIndex, RSMI_CLK_TYPE_SYS, &freqs);
    checkReturn(ret, "Get GPU clock");
    std::cout << "GPU clock: Current = " << freqs.current << " MHz, Max = " << 
    (freqs.num_supported > 0 ? freqs.frequency[freqs.num_supported - 1] : 0) << " MHz" << std::endl;

    ret = rsmi_dev_gpu_clk_freq_get(deviceIndex, RSMI_CLK_TYPE_MEM, &freqs);
    checkReturn(ret, "Get Memory clock");
    std::cout << "Memory clock: Current = " << freqs.current << " MHz, Max = " << 
    (freqs.num_supported > 0 ? freqs.frequency[freqs.num_supported - 1] : 0) << " MHz" << std::endl;

    // Show GPU usage
    uint32_t usage;
    ret = rsmi_dev_busy_percent_get(deviceIndex, &usage);
    checkReturn(ret, "Get GPU usage");
    std::cout << "GPU usage: " << usage << "%" << std::endl;

    // Show energy consumption
    uint64_t power;
    ret = rsmi_dev_power_ave_get(deviceIndex, 0, &power);
    checkReturn(ret, "Get power consumption");
    std::cout << "Power consumption: " << power/1000000.0 << " W" << std::endl;

    // Display total and used memory
    uint64_t total, used;
    ret = rsmi_dev_memory_total_get(deviceIndex, RSMI_MEM_TYPE_VRAM, &total);
    checkReturn(ret, "Get total memory");
    ret = rsmi_dev_memory_usage_get(deviceIndex, RSMI_MEM_TYPE_VRAM, &used);
    checkReturn(ret, "Get used memory");
    std::cout << "Memory usage: " << used/1024/1024 << " MB / " << total/1024/1024 << " MB" << std::endl;


    // Show PCIe bandwidth
    rsmi_pcie_bandwidth_t bw;
    ret = rsmi_dev_pci_bandwidth_get(deviceIndex, &bw);
    checkReturn(ret, "Get PCIe bandwidth");

    // Find the current bandwidth settings
    uint32_t current_rate = 0;
    uint32_t current_lanes = 0;
    for (uint32_t i = 0; i < 32; ++i) {  // Assuming 32 is the size of the array
        if (bw.transfer_rate.current == bw.transfer_rate.frequency[i]) {
            current_rate = bw.transfer_rate.frequency[i];
            current_lanes = bw.lanes[i];  // Directly access the array
            break;
        }
    }

    std::cout << "PCIe Bandwidth: " << current_rate << " GT/s, "
            << current_lanes << " lanes" << std::endl;

    // Show overclocking status
    uint32_t oc_state;
    ret = rsmi_dev_overdrive_level_get(deviceIndex, &oc_state);
    checkReturn(ret, "Get overdrive level");
    std::cout << "Overdrive Level: " << oc_state << std::endl;

    // Display driver information
    char driver_version[256];
    ret = rsmi_version_str_get(RSMI_SW_COMP_DRIVER, driver_version, sizeof(driver_version));
    checkReturn(ret, "Get driver version");
    std::cout << "Driver Version: " << driver_version << std::endl; 
}

void writeAllGPUsInfo() {
    uint32_t numDevices;
    rsmi_status_t ret = rsmi_num_monitor_devices(&numDevices);
    checkReturn(ret, "Get number of devices");

    std::cout << "Number of GPU devices: " << numDevices << std::endl;

    for (uint32_t i = 0; i < numDevices; ++i) {
        std::cout << "------------------------------------------------------------------------" << std::endl;
        std::cout << std::endl;
        std::cout << "========================================================================" << std::endl;
        std::cout << ": GPU " << i << std::endl;
        std::cout << "------------------------------------------------------------------------" << std::endl;
        writeGPUInfo(i);
        std::cout << std::endl;
    }
}

void writeGPUUsage(uint32_t deviceIndex) {
    rsmi_status_t ret;
    uint32_t usage;
    ret = rsmi_dev_busy_percent_get(deviceIndex, &usage);
    checkReturn(ret, "Get GPU usage");
    std::cout << "GPU " << deviceIndex << " Usage: " << usage << "%" << std::endl;
}

void writeGPUTemperature(uint32_t deviceIndex) {
    rsmi_status_t ret;
    int64_t temp;
    ret = rsmi_dev_temp_metric_get(deviceIndex, 0, RSMI_TEMP_CURRENT, &temp);
    checkReturn(ret, "Get temperature");
    std::cout << "GPU " << deviceIndex << " Temperature: " << temp/1000.0 << " °C" << std::endl;
}

void writeHelp() {
    std::cout << "Usage: gpu_info [OPTION]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help         Display this help message" << std::endl;
    std::cout << "  -a, --all          Display all information for all GPUs" << std::endl;
    std::cout << "  -u, --usage        Display GPU usage for all GPUs" << std::endl;
    std::cout << "  -t, --temperature  Display GPU temperature for all GPUs" << std::endl;
}

int main(int argc, char* argv[]) {
    rsmi_status_t ret = rsmi_init(0);
    checkReturn(ret, "Initialize ROCm SMI");

    if (argc == 1 || (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))) {
        writeHelp();
    } else if (argc == 2 && (strcmp(argv[1], "-a") == 0 || strcmp(argv[1], "--all") == 0)) {
        writeAllGPUsInfo();
    } else if (argc == 2 && (strcmp(argv[1], "-u") == 0 || strcmp(argv[1], "--usage") == 0)) {
        uint32_t numDevices;
        ret = rsmi_num_monitor_devices(&numDevices);
        checkReturn(ret, "Get number of devices");
        for (uint32_t i = 0; i < numDevices; ++i) {
            writeGPUUsage(i);
        }
    } else if (argc == 2 && (strcmp(argv[1], "-t") == 0 || strcmp(argv[1], "--temperature") == 0)) {
        uint32_t numDevices;
        ret = rsmi_num_monitor_devices(&numDevices);
        checkReturn(ret, "Get number of devices");
        for (uint32_t i = 0; i < numDevices; ++i) {
            writeGPUTemperature(i);
        }
    } else {
        std::cerr << "Invalid option. Use -h or --help for usage information." << std::endl;
    }

    rsmi_shut_down();
    return 0;
}






