#!/usr/bin/ehi
# One way to implement Singleton
class Singleton {
	private instance = this.new: ()

	private new = func: -> null
	
	public getInstance = func: -> instance
}
printvar: Singleton.getInstance: ()
# error
printvar: Singleton.new: ()
