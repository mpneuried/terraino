$ = require( "jquery" )
require( "jst" )
nunjucks = require( "nunjucks" )
_lang = null

fnRender = ( name, data={} )=>
	
	return nunjucks.render( name, data )

module.exports = fnRender