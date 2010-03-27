#!/bin/sh
if test -f matrix.xml; then
  mv -f matrix.xml matrix.xml~
fi
wget http://translationproject.org/extra/matrix.xml
JAVA_HOME=/arch/x86-linux/inst-java/jdk-1.5.0_06
export JAVA_HOME
PATH="$JAVA_HOME/bin:$PATH"
java -classpath .:jdom-1.0.jar Matrix
