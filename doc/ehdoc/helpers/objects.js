// Read the list of EH standard library classes and generate information about them.
"use strict";

// Ultimately, adds the data about each class into exports.
var glob = require('glob');
var fs = require('fs');

function cleanComment(input) {
	return input.replace(/\s*\*\s*/g, ' ').replace(/^\s*|\s*$/g, '').replace(/\s+/g, ' ');
}

function getTopComment(text, data) {
	var matches = text.match(/^\s*\/\*[\s\*]*[A-Za-z_]+\s*([\s\S]*?)\*\//);
	if(matches !== null) {
		data.description = cleanComment(matches[1]);
	}
}

function processDocComment(comment) {
	var matches = comment.match(/@([a-z]+) ([\s\S]*?)(?=@|\*\/)/g);
	var out = {};
	matches.forEach(function(match) {
		var internalMatch = match.match(/^@([a-z]+) ([\s\S]*)$/);
		var name = internalMatch[1];
		var content = cleanComment(internalMatch[2]);
		out[name] = content;
	});
	return out;
}

function processMethod(name, internalName, text, data) {
	var methodData = {
		name: name,
		returns: "(not given)",
		description: "(not given)",
		argument: "(not given)"
	};
	var regex = new RegExp("(/\\*[\\s\\S]+?\\*/)\\s*EH_METHOD\\(" + data.name + ",\\s*" + RegExp.escape(internalName) + "\\)\\s*{");
	var match = text.match(regex);
	if(match !== null) {
		var descriptions = processDocComment(match[1]);
		methodData.returns = descriptions.returns;
		methodData.description = descriptions.description;
		methodData.argument = descriptions.argument;
	}
	data.methods.push(methodData);
}

glob('../../ehi/std_lib/*.cpp', function(err, matches) {
	if(err) {
		throw err;
	}
	matches.forEach(objectHandler);
});

function objectHandler(path) {
	var name = path.match(/std_lib\/([A-Za-z]+)\.cpp$/)[1];

	var data = {name: name, description: "(not given)", inherits: [], methods: []};
	fs.readFile(path, 'ascii', function(err, content) {
		if(err) {
			throw err;
		}
		getTopComment(content, data);

		var regex = new RegExp("\\nEH_INITIALIZER\\(" + name + "\\) {\\n([\\s\\S]+?)(?=}\\n)");
		var initializer = content.match(regex)[1].split("\n");
		initializer.forEach(function(line) {
			var matches;
			if((matches = line.match(/^\tREGISTER_METHOD\([A-Za-z]+, ([A-Za-z_]+)\);/)) !== null) {
				processMethod(matches[1], matches[1], content, data);
			} else if((matches = line.match(/^\tINHERIT_LIBRARY\(([A-Za-z_]+)\);/)) !== null) {
				data.inherits.push(matches[1]);
			} else if((matches = line.match(/^\tREGISTER_METHOD_RENAME\([A-Za-z]+, ([A-Za-z_]+), "([^\s\"]+)"\);/)) !== null) {
				processMethod(matches[2], matches[1], content, data);
			}
		});
		exports[name] = data;
	});
}
