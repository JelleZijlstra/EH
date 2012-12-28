/*
 * Useful methods added directly to core objects, because that's the EH spirit.
 */
Object.prototype.each = function(f) {
	for(var key in this) {
		if(this.hasOwnProperty(key)) {
			f(key, this[key]);
		}
	}
};
Array.prototype.each = function(f) {
	for(var i = 0, len = this.length; i < len; i++) {
		f(this[i]);
	}
};

// http://stackoverflow.com/questions/3561493/is-there-a-regexp-escape-function-in-javascript/3561711#3561711
RegExp.escape = function(s) {
    return s.replace(/[-\/\\^$*+?.()|[\]{}]/g, '\\$&');
};
