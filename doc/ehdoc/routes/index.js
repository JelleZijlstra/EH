
/*
 * GET home page.
 */

var pages = {
	index: "Home",
	introduction: "Introduction",
	classes: "EH library reference",
	syntax: "Syntax reference",
	magic_methods: "Methods called by the EH engine"
};

var operators = require('../data/operators.json');
var methods = require('../data/methods.json');

var classes = require('../helpers/objects.js');

exports.index = function(req, res) {
	res.render('index', { title: 'Home', pages: pages });
};

exports.introduction = function(req, res) {
	res.render('introduction', {
		title: 'Introduction',
		pages: pages
	});
};

exports.objects = function(req, res) {
	res.render('objects', {
		title: 'Objects',
		pages: pages
	});
};

exports.syntax = function(req, res) {
	res.render('syntax', {
		title: 'Syntax reference',
		pages: pages,
		operators: operators
	});
};

exports.magic_methods = function(req, res) {
	res.render('magic_methods', {
		title: 'Magic methods',
		pages: pages,
		methods: methods
	});
};


exports.classes = function(req, res) {
	if(req.params['class'] === undefined) {
		res.render('classes', {
			title: 'EH class reference',
			pages: pages,
			classes: classes
		});
	} else {
		var name = req.params['class'];
		res.render('class', {
			title: name,
			pages: pages,
			data: classes[name],
			name: name
		});
	}
}
