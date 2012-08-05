#!/usr/bin/ehi
# One way to implement Singleton
class Singleton {
	const instance = this.new ()

	private const new = new
	
	public getInstance = func: -> instance

	const toString = func: -> "@Singleton"
}
printvar Singleton.getInstance ()
# error
printvar Singleton.new ()
