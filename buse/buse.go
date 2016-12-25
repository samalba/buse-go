package buse

import (
	"log"
	"net"
	"os"
	"syscall"
)

// #include <linux/nbd.h>
import "C"

interface BuseInterface {
	ReadAt(p []byte, off int64) (int, error)
	WriteAt(p []byte, off int64) (int, error)
	Disconnect()
	Flush(int, error)
	Trim(int, error)
}

func ioctl(fd, op, arg uintptr) {
	_, _, ep := syscall.Syscall(syscall.SYS_IOCTL, fd, op, arg)
	if ep != 0 {
		log.Fatalf("ioctl(%d, %d, %d) failed: %s", syscall.Errno(ep))
	}
}


func CreateBuseDevice(device string, size uint, buseDriver BuseInterface) error {
	//(domain, typ, proto int) 
	sockPair, err := syscall.Socketpair(syscall.AF_UNIX, syscall.SOCK_STREAM, 0)
	if err != nil {
		log.Fatal("Call to socketpair failed:", err)
	}
	fp, err := os.Create(device)
	if err != nil {
		log.Fatalf("Cannot open \"%s\". Make sure you loaded the `nbd' kernel module: %s", device, err)
	}
	ioctl(fp.Fd(), C.NBD_SET_SIZE, size)
	ioctl(fp.Fd(), C.NBD_CLEAR_SOCK, 0)
	return nil
}
