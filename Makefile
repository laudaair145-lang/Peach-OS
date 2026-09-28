CC = clang
LD = lld-link

CFLAGS = -target x86_64-pc-win32-coff -fno-stack-protector -fshort-wchar -mno-red-zone -ffreestanding -Wall -I.
LDFLAGS = -subsystem:efi_application -nodefaultlib -entry:efi_main

OVMF_SEARCH_PATHS = /usr/share/OVMF/OVMF_CODE.fd \
                    /usr/share/edk2-ovmf/x64/OVMF_CODE.fd \
                    /usr/share/edk2/ovmf/OVMF_CODE.fd \
                    /usr/share/OVMF/OVMF.fd \
                    /usr/share/qemu/OVMF.fd

OVMF_PATH = $(firstword $(wildcard $(OVMF_SEARCH_PATHS)))

all: bootx64.efi

boot.o: boot.c efi.h
	$(CC) $(CFLAGS) -c boot.c -o boot.o

bootx64.efi: boot.o
	$(LD) $(LDFLAGS) boot.o -out:bootx64.efi

run: bootx64.efi
	@if [ -z "$(OVMF_PATH)" ]; then \
		echo "Error: OVMF firmware not found."; \
		exit 1; \
	fi
	@echo "Using OVMF Firmware at: $(OVMF_PATH)"
	mkdir -p fat/EFI/BOOT
	cp bootx64.efi fat/EFI/BOOT/BOOTX64.EFI
	qemu-system-x86_64 -bios $(OVMF_PATH) -net none -drive file=fat:rw:fat,format=raw,media=disk

clean:
	rm -rf boot.o bootx64.efi fat