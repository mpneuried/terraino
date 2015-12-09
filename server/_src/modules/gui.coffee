# # GUI
# ### extends [API Base](../lib/apibase.coffee.html)
#
# ### Exports: *Instance*
# 
# Module to handle the frontend rendering

# **node modules**
querystring = require( "querystring" ) 

# **npm modules**
_ = require( "lodash" )

# **internal modules**
# [config](../lib/config.coffee.html)
config = require( "../lib/config" )
# [User Model](../model/users.coffee.html)
userM = require( "../model/users" )

class GUI extends require( "../lib/apibase" )

	# ## defaults
	defaults: =>
		@extend super, {}

	###
	## createRoutes
	
	`gui.createRoutes( basepath, router )`
	
	The basic method to add routes to express.
	
	@param { String } basepath Basic path prefix 
	@param { Express } router The express app instance 
	
	@api private
	###
	createRoutes: ( basepath, router )=>
		
		# main pages
		router.get "#{basepath}index.html", @index
		router.get "#{basepath}index", @index
		router.get "#{basepath}", @index
		
		
		super
		return

	###
	## index
	
	`gui.index( req, res )`
	
	Render the index page
	
	@param { Request } req Express Request 
	@param { Response } res Express Response 
	
	@api private
	###
	index: (req, res)=>
		

		# get the user
		userM.get "bbbbb", ( err, userdata )=>
			if err
				@_error( res, err )
				return

			_tmpl = 
				title: config.get( "server" ).title
				user: userdata
				# stringify frontend init data to be able to parse it within the template
				init: JSON.stringify
					user: userdata

			res.render('index', _tmpl )
			return
		return



# create the instance and export it.
module.exports = new GUI()