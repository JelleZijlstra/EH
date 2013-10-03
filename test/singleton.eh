#!/usr/bin/ehi
# One way to implement Singleton
class Singleton
	const static instance = this()

	private static const operator() = this.operator()

	public static getInstance() = this.instance

	const toString = () => "@Singleton"
end
printvar(Singleton.getInstance ())
# error
printvar(Singleton())
