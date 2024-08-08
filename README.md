# JVMX2

Java 7 Virtual Machine for Learning Puroposes. It is written in C++11. It uses [GNU Classpath](https://www.gnu.org/software/classpath/home.html), [Boost](https://www.boost.org/) v1.67.0 and [wallaroo](https://wallaroolib.sourceforge.net/index.html) v0.7.0

## Why Another JVM?

This project was something I wrote because I wanted to challenge myself and because I wanted to learn about Virtual Machines.

## Why make this public?

This project is public so that anyone who is interested in learning about Virtual Machines can use it as resource.

## Why call it JVMX2?

I didn't want to call it "Yet another JVM". This is the second incarnation of the project, hence the "2" at the end.

## Why Java 7?

When I started the project, Java 7 was the latest version. Because this is only a pet project for learning, I have never felt the need to upgrade.

## Does JVMX2 run on x64 architecture

JVMX2 is a 32 bit application for now, it will run on x64 but it will use a maximum of 2GB of ram. The main reason for this is that I still need to do the work for the Java Native Interface on x64 architectures. Pull requests are welcome.

## Does JVMX2 run on Linux?

Not at the moment. My focus has been getting it working on my development machine, which is Windows. There is an OS abstraction layer and everything is meant to be portable, but no porting has happened. Pull requests are welcome.

## GNU Classpath

This project requires the [GNU Classpath](https://www.gnu.org/software/classpath/home.html) to run. I am distributing a compiled version of the classpath along with the JVMX2 binary. Please see the license file in the `classpath` folder for what you can do with GNU Classpath.

## Getting Started

You can run `JVMX2 Tests/TestThreads.class`, for example, to test the JVM.

