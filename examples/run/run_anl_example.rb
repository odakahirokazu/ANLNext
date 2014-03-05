#!/usr/bin/env ruby

require 'ANLLib'
require 'myPackage'

include MyPackage

a = ANLApp.new
a.chain :MyModule
a.run(1000000, 100000)
