#!/bin/sh
if test -f matrix.xml; then
  mv -f matrix.xml matrix.xml~
fi
wget http://translationproject.org/extra/matrix.xml
java -classpath .:jdom-1.0.jar Matrix
