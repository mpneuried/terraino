
_ = require( "lodash" )

config = require( "../lib/config" )
utils = require( "../lib/utils" )


class ModelUser extends require( "./_pg_base" )

	ERRORS: =>
		@extend super, 
			"ENOTFOUND": [ 404, "User not found." ]

module.exports = new ModelUser()