#include <jni.h>
#include <string>

#include "libusb_utils.h"
#include <assert.h>

std::string connect_device(int fileDescriptor) {
    libusb_device_handle *devh = nullptr;

    libusb_set_option(nullptr, LIBUSB_OPTION_NO_DEVICE_DISCOVERY, NULL);
    libusb_init(nullptr);

    if (libusb_wrap_sys_device(nullptr, (intptr_t)fileDescriptor, &devh) != LIBUSB_SUCCESS || devh == nullptr) {
        return "Failed to open USB device";
    }

    auto device = libusb_get_device(devh);
    print_device(device, devh);

    // Don't reset the device on Android
    // libusb_reset_device(devh);

    return get_device_name(device, devh);
}

void setVolume(int fileDescriptor, unsigned char *data) {
    libusb_device_handle *devh = nullptr;

    if (libusb_wrap_sys_device(nullptr, (intptr_t)fileDescriptor, &devh) != LIBUSB_SUCCESS || devh == nullptr) {
        return;
    }

    // Samsung One UI 8.5 crashes here.
    // Android doesn't require detaching the kernel driver.
    // libusb_detach_kernel_driver(devh, 0);

    int result = libusb_claim_interface(devh, 0);

    if (result == LIBUSB_SUCCESS) {

        libusb_control_transfer(
                devh,
                0x21,
                0x01,
                0x0201,
                0x0200,
                data,
                2,
                500);

        libusb_control_transfer(
                devh,
                0x21,
                0x01,
                0x0202,
                0x0200,
                data,
                2,
                500);

        libusb_release_interface(devh, 0);
    }

    // Resetting the device isn't needed and may break Samsung.
    // libusb_reset_device(devh);

    libusb_close(devh);
}

unsigned char* as_unsigned_char_array(JNIEnv *env, jbyteArray array) {
    int len = env->GetArrayLength(array);
    unsigned char* buf = new unsigned char[len];
    env->GetByteArrayRegion(array, 0, len, reinterpret_cast<jbyte*>(buf));
    return buf;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_libusbAndroidTest_MainActivity_initializeNativeDevice(
        JNIEnv *env,
        jobject /* this */,
        jint fileDescriptor) {

    std::string deviceName = connect_device(fileDescriptor);

    return env->NewStringUTF(deviceName.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_libusbAndroidTest_MainActivity_setDeviceVolume(
        JNIEnv *env,
        jobject /* this */,
        jint fileDescriptor,
        jbyteArray volume) {

    unsigned char *buffer = as_unsigned_char_array(env, volume);

    setVolume(fileDescriptor, buffer);

    delete[] buffer;
}
