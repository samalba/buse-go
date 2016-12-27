package main

import (
	"signal"
	"os"

	"git.zaphod.us/samalba/buse-go/buse"
)

// This device is an example implementation of an in-memory block device

type DeviceExample struct {
	dataset []byte
}

type ()

func main() {
	size := 1024 * 1024 * 512 // 512M
	deviceExp := &DeviceExample{}
	deviceExp.dataset = make([]byte, size)
	device := CreateDevice("/dev/nbd0", size, deviceExp)
	device.Connect()
	sig := make(chan os.Signal, 1)
	signal.Notify(sig, os.Interrupt)
	<-sig

}
