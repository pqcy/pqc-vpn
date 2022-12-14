export QTDIR=/opt/Qt/5.15.2/gcc_64

#
# lib
#
cd lib
qmake
make -j$(NPROC)
cd ..

#
# app
#
cd app
qmake
make -j$(NPROC)
cd ..

cd setup

rm -rf linux
mkdir -p linux
cd linux

#
# executable files 
#
cp ../../bin/start.sh .
cp ../../bin/stop.sh .
mkdir -p crt
cp ../../bin/crt/* crt/
cp ../../bin/pqcclient-test .; strip pqcclient-test
cp ../../bin/pqcserver-test .; strip pqcserver-test
cp ../../bin/tcpclient-test .; strip tcpclient-test
cp ../../bin/tcpserver-test .; strip tcpserver-test
cp ../../bin/tlsclient-test .; strip tlsclient-test
cp ../../bin/tlsserver-test .; strip tlsserver-test
cp ../../bin/vpnclient-test .; strip vpnclient-test
cp ../../bin/vpnserver-test .; strip vpnserver-test
cp ../../setup/setup-linux.sh .
cp ../openssl/lib*.so* .; strip lib*.so*

#
# qt files
#
cp $QTDIR/lib/libQt5Core.so.5 .
cp $QTDIR/lib/libicui18n.so.56 .
cp $QTDIR/lib/libicuuc.so.56 .
cp $QTDIR/lib/libicudata.so.56 .

#
# platforms files
#
mkdir -p platforms
cp $QTDIR/plugins/platforms/* platforms/

#
# compress
#
tar czf ../pqc-vpn-$(sed 's/"//g' ../../version.txt).tar.gz *
cd ..
cd ..

