#!/bin/sh

test -f jdom-1.0.jar || wget https://repo1.maven.org/maven2/jdom/jdom/1.0/jdom-1.0.jar

JAVA_HOME=/arch/x86-linux/inst-java/jdk-1.5.0_06
export JAVA_HOME
PATH="$JAVA_HOME/bin:$PATH"

javac -d . -classpath jdom-1.0.jar Matrix.java
