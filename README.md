# Go-native block device in user space

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
