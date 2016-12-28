package main

import (
	"fmt"
	"log"
	"os"
	"os/signal"

	"git.zaphod.us/samalba/buse-go/buse"
)

// This device is an example implementation of an in-memory block device

type DeviceExample struct {
	dataset []byte
}

func (d *DeviceExample) ReadAt(p []byte, off uint) error {
	copy(p, d.dataset[off:int(off)+len(p)])
	log.Printf("[DeviceExample] READ offset:%d len:%d\n", off, len(p))
	return nil
}

func (d *DeviceExample) WriteAt(p []byte, off uint) error {
	copy(d.dataset[off:], p)
	log.Printf("[DeviceExample] WRITE offset:%d len:%d\n", off, len(p))
	return nil
}

func (d *DeviceExample) Disconnect() {
	log.Println("[DeviceExample] DISCONNECT")
}

func (d *DeviceExample) Flush() error {
	log.Println("[DeviceExample] FLUSH")
	return nil
}

func (d *DeviceExample) Trim(off, length uint) error {
	log.Printf("[DeviceExample] TRIM offset:%d len:%d\n", off, length)
	return nil
}

func main() {
	size := uint(1024 * 1024 * 512) // 512M
	deviceExp := &DeviceExample{}
	deviceExp.dataset = make([]byte, size)
	device, err := buse.CreateDevice("/dev/nbd10", size, deviceExp)
	if err != nil {
		fmt.Printf("Cannot create device: %s\n", err)
		os.Exit(1)
	}
	sig := make(chan os.Signal, 1)
	signal.Notify(sig, os.Interrupt)
	go func() {
		if err := device.Connect(); err != nil {
			log.Printf("Buse device stopped with error: %s", err)
		} else {
			log.Println("Buse device stopped gracefully.")
		}
	}()
	<-sig
	// Received SIGTERM, cleanup
	device.Disconnect()
}
