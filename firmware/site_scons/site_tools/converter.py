#!/usr/bin/env python
# -*- coding: utf-8 -*
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import os
import subprocess
import xml.etree.ElementTree as ET

import SCons.Builder
import SCons.Action

__author__ = 'andreas.dahlberg90@gmail.com (Andreas Dahlberg)'
__version__ = '0.1.0'


def get_test_file_name(xml_file_name):
    test_file_path = xml_file_name.replace('build/test', 'firmware/src/modules')
    test_file_path = test_file_path.replace('test_result.xml', '')

    module_name = os.path.basename(os.path.normpath(test_file_path))
    test_file_name = 'test_{}.c'.format(module_name)
    return os.path.join(test_file_path, 'test', test_file_name)


def convertXML(input_file, test_file):
    file = ET.Element('file')
    file.set('path', test_file)

    xml_data = None
    with open(input_file, 'r') as xml_file:
        xml_data = xml_file.read()

    #Workaround for cmocka issue with multiple root elements.
    xml_data = xml_data.replace('</testsuites>\n<testsuites>', '')
    root = ET.fromstring(xml_data)

    for testsuite in root:
        for testcase in testsuite:
            name = testcase.attrib['name']
            time_ms = int(float(testcase.attrib['time'])*1000)

            sonar_testcase = ET.Element('testCase')
            sonar_testcase.set('name', name)
            sonar_testcase.set('duration', str(time_ms))
            file.append(sonar_testcase)

            failure = testcase.find('failure')
            if failure is not None:
                failure_message = failure.text
                sonar_failure = ET.Element('failure')
                sonar_failure.set('message', 'Test failure')
                sonar_failure.text = failure_message
                sonar_testcase.append(sonar_failure)

    return file


def generateReport(xml_files, report_file):
    root = ET.Element('testExecutions')
    root.set('version', '1')

    for xml_file in xml_files:
        file_element = convertXML(xml_file, get_test_file_name(xml_file))
        root.append(file_element)

    xml_data = ET.tostring(root, encoding='unicode')
    with open(report_file, 'w') as output_file:
        output_file.write(xml_data)


def build_function(target, source, env):
    """Converts the supplied reports in 'source' to a single Generic Test Execution report."""

    xml_files = [n.path for n in source if n.path.endswith('.xml')]
    generateReport(xml_files, 'report.xml')

    return None


def _get_converter_builder():
    return SCons.Builder.Builder(
        action=SCons.Action.Action(build_function)
    )


def generate(env):
    """Add builders and construction variables to the environment."""

    env.Append(BUILDERS={
        'Converter': _get_converter_builder()
    })


def exists(env):
    return True
