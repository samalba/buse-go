# Linux block device in user space in Golang

## How to use it

Checkout the file driver_example.go for a simple in-memory block device.

Here is how to test, open a terminal:

```
go build
sudo modprobe nbd
sudo ./buse-go /dev/nbd0
```

And in another terminal:

```
mkfs.ext4 /dev/nbd0
mkdir /mnt/test
mount /dev/nbd0 /mnt/test
echo it works > /mnt/test/foo
```

You can check out the logs in the first terminal...

## How does it work?

It uses NBD (Network Block Device) behind the scene. A NBD server and client is automatically setup on the same machine. This project has been inspired by [BUSE in C](https://github.com/acozzette/BUSE).
