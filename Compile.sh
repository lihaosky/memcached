#!/bin/bash
dmcs MonoIsis.cs Isis.cs -r:System.Data.dll -r:System.Web.Extensions.dll -r:Mono.Data.Sqlite.dll -d:MONO_MODE
make
