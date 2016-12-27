package buse

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"log"
	"os"
	"syscall"
	"unsafe"
)

// Rewrote type definitions for #defines and structs to workaround cgo
// as defined in <linux/nbd.h>

const (
	NBD_SET_SOCK        = (0xab<<8 | 0)
	NBD_SET_BLKSIZE     = (0xab<<8 | 1)
	NBD_SET_SIZE        = (0xab<<8 | 2)
	NBD_DO_IT           = (0xab<<8 | 3)
	NBD_CLEAR_SOCK      = (0xab<<8 | 4)
	NBD_CLEAR_QUE       = (0xab<<8 | 5)
	NBD_PRINT_DEBUG     = (0xab<<8 | 6)
	NBD_SET_SIZE_BLOCKS = (0xab<<8 | 7)
	NBD_DISCONNECT      = (0xab<<8 | 8)
	NBD_SET_TIMEOUT     = (0xab<<8 | 9)
	NBD_SET_FLAGS       = (0xab<<8 | 10)
)

const (
	NBD_CMD_READ  = 0
	NBD_CMD_WRITE = 1
	NBD_CMD_DISC  = 2
	NBD_CMD_FLUSH = 3
	NBD_CMD_TRIM  = 4
)

const (
	NBD_FLAG_HAS_FLAGS  = (1 << 0)
	NBD_FLAG_READ_ONLY  = (1 << 1)
	NBD_FLAG_SEND_FLUSH = (1 << 2)
	NBD_FLAG_SEND_TRIM  = (1 << 5)
)

const (
	NBD_REQUEST_MAGIC = 0x25609513
	NBD_REPLY_MAGIC   = 0x67446698
)

type nbdRequest struct {
	magic  uint32
	typ    uint32
	handle [8]byte
	from   uint64
	length uint32
}

type nbdReply struct {
	magic  uint32
	err    uint32
	handle [8]byte
}

type BuseInterface interface {
	ReadAt(p []byte, off int64) (int, error)
	WriteAt(p []byte, off int64) (int, error)
	Disconnect()
	Flush(int, error)
	Trim(int, error)
}

var Endian binary.ByteOrder

func init() {
	var i int = 0x1
	byteList := (*[unsafe.Sizeof(0)]byte)(unsafe.Pointer(&i))
	if byteList[0] == 0 {
		Endian = binary.BigEndian
	} else {
		Endian = binary.BigEndian
	}
}

func ioctl(fd, op, arg uintptr) {
	_, _, ep := syscall.Syscall(syscall.SYS_IOCTL, fd, op, arg)
	if ep != 0 {
		log.Fatalf("ioctl(%d, %d, %d) failed: %s", syscall.Errno(ep))
	}
}

func startNBDClient(deviceFd uintptr, serverSocket int) {
	ioctl(deviceFd, NBD_SET_SOCK, uintptr(serverSocket))
	// The call below may fail on some systems (if flags unset), could be ignored
	ioctl(deviceFd, NBD_SET_FLAGS, NBD_FLAG_SEND_TRIM)
	// The following call will block until the client disconnects
	log.Println("Starting NBD client...")
	ioctl(deviceFd, NBD_DO_IT, 0)
	log.Println("NBD client disconnected")
	ioctl(deviceFd, NBD_CLEAR_QUE, 0)
	ioctl(deviceFd, NBD_CLEAR_SOCK, 0)
}

func CreateBuseDevice(device string, size uint, buseDriver BuseInterface) error {
	sockPair, err := syscall.Socketpair(syscall.AF_UNIX, syscall.SOCK_STREAM, 0)
	if err != nil {
		return fmt.Errorf("Call to socketpair failed: %s", err)
	}
	fp, err := os.Create(device)
	if err != nil {
		return fmt.Errorf("Cannot open \"%s\". Make sure you loaded the `nbd' kernel module: %s", device, err)
	}
	ioctl(fp.Fd(), NBD_SET_SIZE, uintptr(size))
	ioctl(fp.Fd(), NBD_CLEAR_SOCK, 0)
	go startNBDClient(fp.Fd(), sockPair[1])

	//opens the device file at least once, to make sure the partition table is updated
	tmp, err := os.Open(device)
	if err != nil {
		return fmt.Errorf("Cannot reach the device %s: %s", device, err)
	}
	tmp.Close()

	request := nbdRequest{}
	reply := nbdReply{magic: NBD_REPLY_MAGIC}
	fp = os.NewFile(uintptr(sockPair[0]), "unix")
	buf := make([]byte, unsafe.Sizeof(request))

	for true {
		n, err := fp.Read(buf)
		if err != nil {
			log.Println("NBD server stopped:", err)
			return nil
		}
		bufR := bytes.NewReader(buf)
		err = binary.Read(bufR, Endian, &request)
		if err != nil {
			log.Println("Received invalid NBD request:", err)
		}
		chunk := make([]byte, request.length)
		if request.typ == NBD_CMD_READ {
			n, err = buseDriver.ReadAt(chunk, int64(request.from))
			if err != nil {
				log.Println("buseDriver.ReadAt returned an error:", err)
				// Reply with an EPERM
				reply.err = 1
			} else {
				reply.err = uint32(n)
			}
			bufB := new(bytes.Buffer)
			if err := binary.Write(bufB, Endian, reply); err != nil {
				return fmt.Errorf("Fatal error, cannot write reply packet: %s", err)
			}
			if _, err := fp.Write(bufB.Bytes()); err != nil {
				log.Println("Write error, when sending reply header:", err)
			}
			if _, err := fp.Write(chunk); err != nil {
				log.Println("Write error, when sending data chunk:", err)
			}
		}
	}
	return nil
}
