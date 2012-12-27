// An EH module for the SyntaxHighlighter JavaScript plugin
// See http://alexgorbatchev.com/SyntaxHighlighter/

// Originally copied from shBrushPhp.js
;(function()
{
	// CommonJS
	typeof(require) != 'undefined' ? SyntaxHighlighter = require('shCore').SyntaxHighlighter : null;

	function Brush()
	{
		// Functions that have a special meaning for the interpreter, not just ones that do something special
		var funcs = 'toString toFloat toInt toBool toRange toArray initialize finalize operator_plus operator_minus operator_times operator_divide operator_modulo operator_and operator_or operator_xor operator_tilde operator_uminus operator_colon operator_arrow operator_arrow_equals';

		var keywords = 'if else set endif for in endfor as count while endwhile break continue func endfunc ret class endclass inherit switch endswitch given end case default and or xor try catch finally public private static const';

		var constants = 'true false null';

		this.regexList = [
			{ regex: SyntaxHighlighter.regexLib.singleLineCComments,	css: 'comments' },			// one line comments
			{ regex: SyntaxHighlighter.regexLib.singleLinePerlComments,	css: 'comments' },		// one line comments
			{ regex: SyntaxHighlighter.regexLib.doubleQuotedString,		css: 'string' },			// double quoted strings
			{ regex: SyntaxHighlighter.regexLib.singleQuotedString,		css: 'string' },			// single quoted strings
			{ regex: new RegExp(this.getKeywords(funcs), 'gm'),		css: 'functions' },			// common functions
			{ regex: new RegExp(this.getKeywords(constants), 'gm'),	css: 'constants' },			// constants
			{ regex: new RegExp(this.getKeywords(keywords), 'gm'),		css: 'keyword' },			// keyword
			{ regex: /\.\w+/gm, css: 'constants' } // object members
			];
	};

	Brush.prototype	= new SyntaxHighlighter.Highlighter();
	Brush.aliases = ['eh'];

	SyntaxHighlighter.brushes.Eh = Brush;

	// CommonJS
	typeof(exports) != 'undefined' ? exports.Brush = Brush : null;
})();
