#!/usr/bin/python3

import os
import sys
import xml.etree.ElementTree


class Interface:
    def __init__(self, name, version, requests, events, enums):
        self.name = name
        self.version = version
        self.requests = requests
        self.events = events
        self.enums = enums


class Request:
    def __init__(self, name, args):
        self.name = name
        self.args = args


class Event:
    def __init__(self, name, args):
        self.name = name
        self.args = args


class Arg:
    def __init__(self, name, type, enum, interface):
        self.name = name
        self.type = type
        self.enum = enum
        self.interface = interface


class Enum:
    def __init__(self, name, entries):
        self.name = name
        self.entries = entries


class EnumEntry:
    def __init__(self, name, value):
        self.name = name
        self.value = value


if len(sys.argv) < 3:
    print("Usage generate-wayland.py [protocol.xml] [server|client] [interfaces]")
    sys.exit(1)
protocol_path = sys.argv[1]
generate_type = sys.argv[2]
interface_names = sys.argv[3:]

if not generate_type in ("server", "client"):
    print("Only server and client output supported")
    sys.exit(1)

interfaces = []
tree = xml.etree.ElementTree.parse(protocol_path)
root = tree.getroot()
for interface in tree.findall("interface"):
    requests = []
    for request in interface.findall("request"):
        args = []
        for arg in request.findall("arg"):
            args.append(
                Arg(
                    arg.attrib["name"],
                    arg.attrib["type"],
                    arg.attrib.get("enum", None),
                    arg.attrib.get("interface", None),
                )
            )
        requests.append(Request(request.attrib["name"], args))
    events = []
    for event in interface.findall("event"):
        args = []
        for arg in event.findall("arg"):
            args.append(
                Arg(
                    arg.attrib["name"],
                    arg.attrib["type"],
                    arg.attrib.get("enum", None),
                    arg.attrib.get("interface", None),
                )
            )
        events.append(Event(event.attrib["name"], args))
    enums = []
    for enum in interface.findall("enum"):
        entries = []
        for entry in enum.findall("entry"):
            entries.append(
                EnumEntry(
                    entry.attrib["name"],
                    entry.attrib["value"],
                )
            )
        enums.append(Enum(enum.attrib["name"], entries))
    interfaces.append(
        Interface(
            interface.attrib["name"],
            interface.attrib["version"],
            requests,
            events,
            enums,
        )
    )


def snake_to_camel(name):
    camel_name = ""
    is_start = True
    for c in name:
        if c == "_":
            is_start = True
        else:
            if is_start:
                camel_name += c.upper()
                is_start = False
            else:
                camel_name += c
    return camel_name


def type_to_native(type):
    return {
        "int": "int32_t",
        "uint": "uint32_t",
        "fixed": "double",
        "string": "const char *",
        "object": "uint32_t",
        "new_id": "uint32_t",
        "array": "uint32_t*",
        "fd": "int",
    }[type]


def arg_to_native(arg):
    args = []
    if arg.type == "new_id" and arg.interface is None:
        args.append("const char *%s_interface" % arg.name)
        args.append("uint32_t %s_version" % arg.name)
    args.append("%s %s" % (type_to_native(arg.type), arg.name))
    return args


def generate_server(interface):
    header_path = interface.name + "_server.h"
    source_path = interface.name + "_server.c"
    class_name = snake_to_camel(interface.name) + "Server"
    prefix = "%s_server" % interface.name
    callbacks_struct = "%sRequestCallbacks" % class_name

    header = ""
    header += "typedef struct _%s %s;\n" % (class_name, class_name)
    header += "\n"
    header += "#pragma once\n"
    header += "\n"
    header += "#include <stdint.h>\n"
    header += "\n"
    header += '#include "wayland_server_client.h"\n'
    for enum in interface.enums:
        enum_prefix = "%s_server_%s" % (interface.name, enum.name)
        enum_name = snake_to_camel(enum_prefix)
        header += "\n"
        header += "typedef enum {\n"
        for entry in enum.entries:
            header += "  %s_%s = %s," % (
                enum_prefix,
                entry.name,
                entry.value,
            )
        header += "} %s;\n" % enum_name
    if len(interface.requests) > 0:
        header += "\n"
        header += "typedef struct {\n"
        for request in interface.requests:
            args = []
            for arg in request.args:
                args.extend(arg_to_native(arg))
            args.append("void *user_data")
            header += "  void (*%s)(%s);\n" % (request.name, ",".join(args))
        header += "} %s;\n" % callbacks_struct
    header += "\n"
    args = ["WaylandServerClient *client", "uint32_t id"]
    if len(interface.requests) > 0:
        args.extend(
            [
                "const %s *request_callbacks" % callbacks_struct,
                "void *user_data",
                "void (*user_data_unref)(void *)",
            ]
        )
    header += "%s *%s_new(%s);\n" % (class_name, prefix, ",".join(args))
    header += "\n"
    header += "%s *%s_ref(%s *self);\n" % (class_name, prefix, class_name)
    header += "\n"
    header += "void %s_unref(%s *self);\n" % (prefix, class_name)
    for event in interface.events:
        header += "\n"
        args = ["%s *self" % class_name]
        for arg in event.args:
            args.extend(arg_to_native(arg))
        header += "void %s_%s(%s);\n" % (prefix, event.name, ",".join(args))

    source = ""
    source += "#include <stdlib.h>\n"
    source += "\n"
    source += '#include "%s"\n' % header_path
    source += "\n"
    source += "struct _%s {\n" % class_name
    source += "  WaylandServerClient *client;\n"
    source += "  uint32_t id;\n"
    if len(interface.requests) > 0:
        source += "  const %s *request_callbacks;\n" % callbacks_struct
        source += "  void *user_data;\n"
        source += "  void (*user_data_unref)(void *);\n"
    source += "};\n"
    for request in interface.requests:
        source += "\n"
        source += "static void %s_%s(%s *self, WaylandMessageDecoder *decoder) {\n" % (
            interface.name,
            request.name,
            class_name,
        )
        args = []
        for arg in request.args:
            if arg.type == "new_id" and arg.interface is None:
                source += (
                    "  const char *%s_interface = wayland_message_decoder_read_string(decoder);\n"
                    % arg.name
                )
                source += (
                    "  uint32_t %s_version = wayland_message_decoder_read_uint(decoder);\n"
                    % arg.name
                )
                args.append("%s_interface" % arg.name)
                args.append("%s_version" % arg.name)
            source += "  %s %s = wayland_message_decoder_read_%s(decoder);\n" % (
                type_to_native(arg.type),
                arg.name,
                arg.type,
            )
            args.append("%s" % arg.name)
        source += "  if (!wayland_message_decoder_finish(decoder)) {\n"
        source += "    // FIXME\n"
        source += "    return;\n"
        source += "  }\n"
        args.append("self->user_data")
        source += "  self->request_callbacks->%s(%s);\n" % (
            request.name,
            ",".join(args),
        )
        source += "}\n"
    source += "\n"
    source += (
        "static void %s_request_cb(WaylandMessageDecoder *decoder, void *user_data) {\n"
        % interface.name
    )
    if len(interface.requests) > 0:
        source += "  %s *self = user_data;\n" % class_name
        source += "\n"
        source += "  switch(wayland_message_decoder_get_code(decoder)) {\n"
        for code, request in enumerate(interface.requests):
            source += "  case %d:\n" % code
            source += "    %s_%s(self, decoder);\n" % (
                interface.name,
                request.name,
            )
            source += "    break;\n"
        source += "  }\n"
    source += "}\n"
    source += "\n"
    args = ["WaylandServerClient *client", "uint32_t id"]
    if len(interface.requests) > 0:
        args.extend(
            [
                "const %s *request_callbacks" % callbacks_struct,
                "void *user_data",
                "void (*user_data_unref)(void *)",
            ]
        )
    source += "%s *%s_new(%s) {\n" % (class_name, prefix, ",".join(args))
    source += "  %s *self = malloc(sizeof(%s));\n" % (class_name, class_name)
    source += "  self->client = client;\n"
    source += "  self->id = id;\n"
    if len(interface.requests) > 0:
        source += "  self->request_callbacks = request_callbacks;\n"
        source += "  self->user_data = user_data;\n"
        source += "  self->user_data_unref = user_data_unref;\n"
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
    for code, event in enumerate(interface.events):
        source += "\n"
        args = ["%s *self" % class_name]
        for arg in event.args:
            args.extend(arg_to_native(arg))
        source += "void %s_%s(%s) {\n" % (prefix, event.name, ",".join(args))
        source += (
            "  WaylandMessageEncoder *encoder = wayland_message_encoder_new(self->id, %d);\n"
            % code
        )
        for arg in event.args:
            if arg.type == "new_id" and arg.interface is None:
                source += (
                    "  wayland_message_encoder_write_string(encoder, %s_interface);\n"
                    % arg.name
                )
                source += (
                    "  wayland_message_encoder_write_uint(encoder, %s_version);\n"
                    % arg.name
                )
            source += "  wayland_message_encoder_write_%s(encoder, %s);\n" % (
                arg.type,
                arg.name,
            )
        source += "  if (!wayland_message_encoder_finish(encoder)) {\n"
        source += "    // FIXME\n"
        source += "  }\n"
        source += "\n"
        source += "  wayland_server_client_send_message(self->client, encoder);\n"
        source += "\n"
        source += "  wayland_message_encoder_unref(encoder);\n"
        source += "}\n"

    open(header_path, "w").write(header)
    open(source_path, "w").write(source)
    os.system("clang-format -i %s %s" % (header_path, source_path))
    print("Generated %s %s" % (source_path, header_path))


def generate_client(interface):
    header_path = interface.name + "_client.h"
    source_path = interface.name + "_client.c"
    class_name = snake_to_camel(interface.name) + "Client"
    prefix = "%s_client" % interface.name
    callbacks_struct = "%sEventCallbacks" % class_name

    header = ""
    header += "typedef struct _%s %s;\n" % (class_name, class_name)
    header += "\n"
    header += "#pragma once\n"
    header += "\n"
    header += "#include <stdint.h>\n"
    header += "\n"
    header += '#include "wayland_client.h"\n'
    for enum in interface.enums:
        enum_prefix = "%s_client_%s" % (interface.name, enum.name)
        enum_name = snake_to_camel(enum_prefix)
        header += "\n"
        header += "typedef enum {\n"
        for entry in enum.entries:
            header += "  %s_%s = %s," % (
                enum_prefix,
                entry.name,
                entry.value,
            )
        header += "} %s;\n" % enum_name
    if len(interface.events) > 0:
        header += "\n"
        header += "typedef struct {\n"
        for event in interface.events:
            args = []
            for arg in event.args:
                args.extend(arg_to_native(arg))
            args.append("void *user_data")
            header += "  void (*%s)(%s);\n" % (event.name, ",".join(args))
        header += "} %s;\n" % callbacks_struct
    header += "\n"
    args = ["WaylandClient *client"]
    if len(interface.events) > 0:
        args.extend(
            [
                "const %s *event_callbacks" % callbacks_struct,
                "void *user_data",
                "void (*user_data_unref)(void *)",
            ]
        )
    header += "%s *%s_new(%s);\n" % (class_name, prefix, ",".join(args))
    header += "\n"
    header += "%s *%s_ref(%s *self);\n" % (class_name, prefix, class_name)
    header += "\n"
    header += "void %s_unref(%s *self);\n" % (prefix, class_name)
    header += "\n"
    header += "uint32_t %s_get_id(%s *self);\n" % (prefix, class_name)
    for request in interface.requests:
        header += "\n"
        args = ["%s *self" % class_name]
        for arg in request.args:
            args.extend(arg_to_native(arg))
        header += "void %s_%s(%s);\n" % (prefix, request.name, ",".join(args))

    source = ""
    source += "#include <stdlib.h>\n"
    source += "\n"
    source += '#include "%s"\n' % header_path
    source += "\n"
    source += "struct _%s {\n" % class_name
    source += "  WaylandClient *client;\n"
    source += "  uint32_t id;\n"
    if len(interface.events) > 0:
        source += "  const %s *event_callbacks;\n" % callbacks_struct
        source += "  void *user_data;\n"
        source += "  void (*user_data_unref)(void *);\n"
    source += "};\n"
    for event in interface.events:
        source += "\n"
        source += "static void %s_%s(%s *self, WaylandMessageDecoder *decoder) {\n" % (
            interface.name,
            event.name,
            class_name,
        )
        args = []
        for arg in event.args:
            if arg.type == "new_id" and arg.interface is None:
                source += (
                    "  const char *%s_interface = wayland_message_decoder_read_string(decoder);\n"
                    % arg.name
                )
                source += (
                    "  uint32_t %s_version = wayland_message_decoder_read_uint(decoder);\n"
                    % arg.name
                )
                args.append("%s_interface" % arg.name)
                args.append("%s_version" % arg.name)
            source += "  %s %s = wayland_message_decoder_read_%s(decoder);\n" % (
                type_to_native(arg.type),
                arg.name,
                arg.type,
            )
            args.append("%s" % arg.name)
        source += "  if (!wayland_message_decoder_finish(decoder)) {\n"
        source += "    // FIXME\n"
        source += "    return;\n"
        source += "  }\n"
        args.append("self->user_data")
        source += "  self->event_callbacks->%s(%s);\n" % (
            event.name,
            ",".join(args),
        )
        source += "}\n"
    source += "\n"
    source += (
        "static void %s_event_cb(WaylandMessageDecoder *decoder, void *user_data) {\n"
        % interface.name
    )
    if len(interface.events) > 0:
        source += "  %s *self = user_data;\n" % class_name
        source += "\n"
        source += "  switch(wayland_message_decoder_get_code(decoder)) {\n"
        for code, event in enumerate(interface.events):
            source += "  case %d:\n" % code
            source += "    %s_%s(self, decoder);\n" % (
                interface.name,
                event.name,
            )
            source += "    break;\n"
        source += "  }\n"
    source += "}\n"
    source += "\n"
    source += "static void unref_object(void *user_data) {\n"
    source += "    %s *self = user_data;\n" % class_name
    source += "    %s_unref(self);\n" % prefix
    source += "}\n"
    source += "\n"
    args = ["WaylandClient *client"]
    if len(interface.events) > 0:
        args.extend(
            [
                "const %s *event_callbacks" % callbacks_struct,
                "void *user_data",
                "void (*user_data_unref)(void *)",
            ]
        )
    source += "%s *%s_new(%s) {\n" % (class_name, prefix, ",".join(args))
    source += "  %s *self = malloc(sizeof(%s));\n" % (class_name, class_name)
    source += "  self->client = client;\n"
    if len(interface.events) > 0:
        source += "  self->event_callbacks = event_callbacks;\n"
        source += "  self->user_data = user_data;\n"
        source += "  self->user_data_unref = user_data_unref;\n"
    source += "\n"
    source += (
        "  self->id = wayland_client_add_object(client, %s_event_cb, NULL, self, unref_object);\n"
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
    source += "\n"
    source += "uint32_t %s_get_id(%s *self) {\n" % (prefix, class_name)
    source += "  return self->id;\n"
    source += "}\n"
    for code, request in enumerate(interface.requests):
        source += "\n"
        args = ["%s *self" % class_name]
        for arg in request.args:
            args.extend(arg_to_native(arg))
        source += "void %s_%s(%s) {\n" % (prefix, request.name, ",".join(args))
        source += (
            "  WaylandMessageEncoder *encoder = wayland_message_encoder_new(self->id, %d);\n"
            % code
        )
        for arg in request.args:
            if arg.type == "new_id" and arg.interface is None:
                source += (
                    "  wayland_message_encoder_write_string(encoder, %s_interface);\n"
                    % arg.name
                )
                source += (
                    "  wayland_message_encoder_write_uint(encoder, %s_version);\n"
                    % arg.name
                )
            source += "  wayland_message_encoder_write_%s(encoder, %s);\n" % (
                arg.type,
                arg.name,
            )
        source += "  if (!wayland_message_encoder_finish(encoder)) {\n"
        source += "    // FIXME\n"
        source += "  }\n"
        source += "\n"
        source += "  wayland_client_send_message(self->client, encoder);\n"
        source += "\n"
        source += "  wayland_message_encoder_unref(encoder);\n"
        source += "}\n"

    open(header_path, "w").write(header)
    open(source_path, "w").write(source)
    os.system("clang-format -i %s %s" % (header_path, source_path))
    print("Generated %s %s" % (source_path, header_path))


for interface in interfaces:
    if len(interface_names) > 0 and interface.name not in interface_names:
        continue

    if generate_type == "server":
        generate_server(interface)
    elif generate_type == "client":
        generate_client(interface)
