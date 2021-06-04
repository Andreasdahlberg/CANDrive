#!/bin/sh

#Download and install build wrapper
wget http://sonarcloud.io/static/cpp/build-wrapper-linux-x86.zip
unzip -j build-wrapper-linux-x86.zip -d /usr/local/bin
cp /usr/local/bin/libinterceptor-x86_64.so /usr/local/bin/libinterceptor-haswell.so
rm build-wrapper-linux-x86.zip

#Download and install SonarScanner
wget https://binaries.sonarsource.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-4.4.0.2170-linux.zip
unzip sonar-scanner-cli-4.4.0.2170-linux.zip
rm sonar-scanner-cli-4.4.0.2170-linux.zip
mv sonar-scanner-4.4.0.2170-linux /usr/lib/sonar-scanner
ln -s /usr/lib/sonar-scanner/bin/sonar-scanner /usr/local/bin/sonar-scanner

#export SONAR_RUNNER_HOME=/usr/lib/sonar-scanner

#Set configuration
cp scripts/sonar-scanner.properties /usr/lib/sonar-scanner/conf/sonar-scanner.properties
