#!/bin/sh
JAVA_HOME=/arch/x86-linux/inst-java/jdk-1.5.0_06
export JAVA_HOME
PATH="$JAVA_HOME/bin:$PATH"
javac -d . -classpath jdom-1.0.jar Matrix.java
