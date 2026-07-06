void setVolume(int fileDescriptor, unsigned char *data) {
    libusb_device_handle *devh = nullptr;

    if (libusb_wrap_sys_device(nullptr, (intptr_t)fileDescriptor, &devh) != LIBUSB_SUCCESS || devh == nullptr) {
        return;
    }

    // Do NOT detach kernel driver on Android/One UI 8.5.
    // It crashes on some Samsung devices.
    // libusb_detach_kernel_driver(devh, 0);

    if (libusb_claim_interface(devh, 0) == LIBUSB_SUCCESS) {

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

    // Don't reset the DAC.
    // libusb_reset_device(devh);

    libusb_close(devh);
}
