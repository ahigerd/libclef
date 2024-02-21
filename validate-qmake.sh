#!/bin/sh
if [ "$QMAKE" == "" ]; then
  QMAKE=qmake
fi

XSPEC=$($QMAKE -query QMAKE_XSPEC 2> /dev/null)
if [ $? -eq 0 ]; then
  if [ "$XSPEC" == "" ]; then
    echo $QMAKE is misconfigured.
    exit 1
  fi
else
  echo Unable to execute qmake binary: $QMAKE
  exit 1
fi

if [ "$1" == "" ]; then
  EXPECT_XSPEC=$($QMAKE -query QMAKE_SPEC)
else
  EXPECT_XSPEC=$1
fi
if [ "$XSPEC" != "$EXPECT_XSPEC" ]; then
  if [ "$MAKE" == "" ]; then
    MAKE=make
  fi
  echo
  echo Incorrect qmake binary detected: $QMAKE
  echo - qmake target platform: "'$XSPEC'"
  echo - expected: "'$EXPECT_XSPEC'"
  echo
  echo Please specify the path to the correct qmake binary:
  echo "  $MAKE gui QMAKE=/path/to/qmake"
  echo
  exit 1
fi
