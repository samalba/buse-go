package buse

type BuseInterface interface {
	ReadAt(p []byte, off uint) error
	WriteAt(p []byte, off uint) error
	Disconnect()
	Flush() error
	Trim(off uint, length uint) error
}
