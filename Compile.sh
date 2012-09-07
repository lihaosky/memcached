#!/bin/bash
dmcs MonoIsis.cs Isis_old1.cs -r:System.Data.dll -r:System.Web.Extensions.dll -r:Mono.Data.Sqlite.dll -d:__MonoCS__
make
