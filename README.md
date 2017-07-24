# RioFS [![Build Status](https://secure.travis-ci.org/skoobe/riofs.png)](https://travis-ci.org/skoobe/riofs) <a href="https://scan.coverity.com/projects/406"><img alt="Coverity Scan Build Status" src="https://scan.coverity.com/projects/406/badge.svg"/></a>

RioFS is an userspace filesystem for Amazon S3 buckets for servers that run on Linux and MacOSX. It supports versioned and non-versioned buckets in all AWS regions. RioFS development started at [Skoobe](https://www.skoobe.de) as a storage backend for legacy daemons which cannot talk natively to S3. It handles buckets with many thousands of keys and highly concurrent access gracefully.  

This particular repository is a fork of the original RioFS with support for IAM roles.  It has an additional dependancy on libcurl.

It also supports a bucket_path_prefix argument and configuration parameter, that can be used to store more
that one filesystem in a bucket. The path prefix should normally end with a slash, otherwise the non-directory nature
of the resulting file system view will be confusing.

### Dependencies

* C compiler
* autoconf
* automake
* glib >= 2.22
* fuse >= 2.7.3
* libevent >= 2.0
* libxml >= 2.6
* libcrypto >= 0.9
* libcurl >= 7.0
* libmagic (optional: --with-libmagic=PATH)

Find here installation guides for [Ubuntu](https://github.com/skoobe/riofs/wiki/Ubuntu), [Centos](https://github.com/skoobe/riofs/wiki/Centos) and [MacOSX](https://github.com/skoobe/riofs/wiki/MacOSX)

### Building

```
./autogen.sh
./configure
make
sudo make install
```

### Using

```
export AWS_ACCESS_KEY_ID="your AWS access key"
export AWS_SECRET_ACCESS_KEY="your AWS secret access key"
- or - 
export AWS_IAM_ROLE="Your IAM role"

riofs [options] [bucketname] [mountpoint]
```

#### Options

```
Usage:
  riofs [OPTION?] [bucketname] [mountpoint]

Help Options:
  -h, --help                                  Show help options

Application Options:
  -c, --config                                Path to configuration file. Default: /usr/local/Cellar/riofs/0.6/etc/riofs/riofs.conf.xml
  --uid                                       Set UID of filesystem owner.
  --gid                                       Set GID of filesystem owner.
  --fmode                                     Set mode for all files.
  --dmode                                     Set mode for all directories.
  -f, --foreground                            Flag. Do not daemonize process.
  --cache-dir                                 Set cache directory.
  -o, --fuse-options="opt[,opt...]"           Fuse options.
  -p, --bucket_prefix_path="opt[,opt...]"     prefix path for mount point within bucket.
  --disable-syslog                            Flag. Disable logging to syslog.
  --disable-stats                             Flag. Disable Statistics HTTP interface.
  --part-size                                 Set file part size (in bytes).
  -l, --log-file                              File to write output.
  --force-head-requests                       Flag. Send HEAD request for each file.
  -v, --verbose                               Verbose output.
  -V, --version                               Show application version and exit.

Please set both AWS_ACCESS_KEY_ID and AWS_SECRET_ACCESS_KEY environment variables,
put then in the config file, or use an IAM role on an EC2 instance!
```

#### Hints

*   In order to allow other users to access a mounted directory:

    - make sure `/etc/fuse.conf` contains `user_allow_other` option
  
    - launch RioFS with  `-o "allow_other"`  parameter

* On OS X it is recommended to run RioFS with the `-o "direct_io"` parameter
 
* Default configuration is located at `$(prefix)/etc/riofs.conf.xml`

* Use `./configure --with-libmagic=PATH` to guess the content-type of uploaded content (requires libmagic)

* Use `./configure --enable-debug` to create a debug build

* RioFS comes with a statistics server, have a look at riofs.xml.conf for details

* Send a USR1 signal to tell RioFS to reread the configuration file

* Send a USR2 signal to tell RioFS to reopen log file (useful for logrotate)

* Send a TERM signal to unmount filesystem and terminate running RioFS instance (example: ```killall riofs```)

### Known limitations

* Appending data to an existing file is not supported.

* Folder renaming is not supported.

* A file system for the S3 API is a [leaky abstraction](http://en.wikipedia.org/wiki/Leaky_abstraction). Don't expect POSIX file system semantics.

### Contribute

* Any help is welcome, just open an issue if you find a bug

* We also need better documentation, testing, tutorials and benchmarks
