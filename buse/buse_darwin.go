package buse

import (
	"encoding/binary"
	"fmt"
	"io"
	"log"
	"os"
	"syscall"
	"unsafe"
)

// Disconnect disconnects the BuseDevice
func (bd *BuseDevice) Disconnect() {
}

// Connect connects a BuseDevice to an actual device file
// and starts handling requests. It does not return until it's done serving requests.
func (bd *BuseDevice) Connect() error {
	return nil
}

func CreateDevice(device string, size uint, blockSize uint, buseDriver BuseInterface) (*BuseDevice, error) {
	return nil, nil
}
