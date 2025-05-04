#!/usr/bin/python3

import os
import xml.etree.ElementTree


class Interface:
    def __init__(self, name, version, requests, events):
        self.name = name
        self.version = version
        self.requests = requests
        self.events = events


class Request:
    def __init__(self, name, args):
        self.name = name
        self.args = args


class Event:
    def __init__(self, name, args):
        self.name = name
        self.args = args


interfaces = []
tree = xml.etree.ElementTree.parse("wayland.xml")
root = tree.getroot()
for interface in tree.findall("interface"):
    requests = []
    for request in interface.findall("request"):
        requests.append(Request(request.attrib["name"], []))
    events = []
    for event in interface.findall("event"):
        events.append(Event(event.attrib["name"], []))
    interfaces.append(
        Interface(
            interface.attrib["name"], interface.attrib["version"], requests, events
        )
    )

def snake_to_camel(name):
    camel_name = ''
    is_start = True
    for c in name:
        if c == '_':
            is_start = True
        else:
            if is_start:
                camel_name += c.upper()
                is_start = False
            else:
                camel_name += c
    return camel_name

for interface in interfaces:
    header_path = interface.name + "_server.h"
    source_path = interface.name + "_server.c"

    class_name = snake_to_camel(interface.name) + 'Server'
    prefix = "%s_server" % interface.name
    callbacks_struct = "%sRequestCallbacks" % class_name

    header = ""
    header += "#pragma once\n"    
    header += "\n"
    header += "#include <stdint.h>\n"
    header += "\n"
    header += '#include "wayland_server_client.h"\n'
    header += "\n"
    header += 'typedef struct {\n'
    for request in interface.requests:
        header += '  void (*%s)(void *user_data);\n' % request.name
    header += '} %s;\n' % callbacks_struct
    header += "\n"    
    header += 'typedef struct _%s %s;\n' % (class_name, class_name)
    header += "\n"
    header += "%s *%s_new(WaylandServerClient *client, uint32_t id, const %s *request_callbacks, void *user_data);\n" % (class_name, prefix, callbacks_struct)
    header += "\n"
    header += "%s *%s_ref(%s *self);\n" % (class_name, prefix, class_name)
    header += "\n"
    header += "void %s_unref(%s *self);\n" % (prefix, class_name)
    for event in interface.events:
        header += "\n"
        header += "void %s_%s(%s *self);\n" % (prefix, event.name, class_name)

    source = ""
    source += "#include <stdlib.h>\n"
    source += "\n"
    source += '#include "%s"\n' % header_path
    source += "\n"
    source += "struct _%s {\n" % class_name
    source += "  const %s *request_callbacks;\n" % callbacks_struct
    source += "  void *user_data;\n"
    source += "};\n"
    for request in interface.requests:
        source += "\n"
        source += (
            "static void %s_%s(%s *self, const uint8_t *payload, uint16_t payload_length) {\n"
            % (interface.name, request.name, class_name)
        )
        source += "  self->request_callbacks->%s(self->user_data);\n" % request.name
        source += "}\n"
    source += "\n"
    source += (
        "static void %s_request_cb(uint16_t code, const uint8_t *payload, uint16_t payload_length, void *user_data) {\n"
        % interface.name
    )
    source += "  %s *self = user_data;\n" % class_name
    source += "\n"
    source += "  switch(code) {\n"
    for i, request in enumerate(interface.requests):
        source += "  case %d:\n" % i
        source += "    %s_%s(self, payload, payload_length);\n" % (
            interface.name,
            request.name,
        )
        source += "    break;\n"
    source += "  }\n"
    source += "}\n"
    source += "\n"
    source += (
        "%s *%s_new(WaylandServerClient *client, uint32_t id, const %s *request_callbacks, void *user_data) {\n"
        % (class_name, prefix, callbacks_struct)
    )
    source += "  %s *self = malloc(sizeof(%s));\n" % (class_name, class_name)
    source += "  self->request_callbacks = request_callbacks;\n"
    source += "  self->user_data = user_data;\n"
    source += "\n"
    source += (
        "  wayland_server_client_add_object(client, id, %s_request_cb, self);\n"
        % interface.name
    )
    source += "\n"
    source += "  return self;\n"
    source += "}\n"
    source += "\n"
    source += "%s *%s_ref(%s *self) {\n" % (class_name, prefix, class_name)
    source += "  // FIXME\n"
    source += "  return self;\n"
    source += "}\n"
    source += "\n"
    source += "void %s_unref(%s *self) {\n" % (prefix, class_name)
    source += "  // FIXME\n"
    source += "}\n"
    for event in interface.events:
        source += "\n"
        source += "void %s_%s(%s *self) {\n" % (prefix, event.name, class_name)
        source += "  // FIXME\n"
        source += "}\n"

    open(header_path, "w").write(header)
    open(source_path, "w").write(source)
    os.system("clang-format -i %s %s" % (header_path, source_path))
