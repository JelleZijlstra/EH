#!/usr/bin/ehi
# One way to implement Singleton
class Singleton {
	const instance = this.new ()

	private const new = this.new
	
	public getInstance = () => this.instance

	const toString = () => "@Singleton"
}
printvar Singleton.getInstance ()
# error
printvar Singleton.new ()
