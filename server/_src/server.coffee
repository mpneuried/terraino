# # Server
# ### extends [NPM:MPBasic](https://github.com/mpneuried/mpbaisc)
#
# ### Exports: *Instance*
# 
# Configure and start the Express server. It also add all the subroutes from the modules

# ### Events
# 
# * **configured**: emitted when the express configure has been done.
# * **loaded**: emitted when all modules has been loaded and attached to express

# **node modules**
path = require( "path" )
http = require( "http" )

# npm modules
_ = require('lodash')

# npm express 4 modules
express = require( "express" )
compression = require('compression')
serveStatic = require('serve-static')
morgan = require('morgan')
bodyParser = require('body-parser')
nunjucks = require('nunjucks')

# **internal modules**
# [Config](./lib/config.coffee.html)
Config = require( "./lib/config" )

class Server extends require( "mpbasic" )( Config )

	# ## defaults
	defaults: =>
		@extend super,
			# **port** *Number* The port the server will listen for.
			port: 5000
			# **host** *String* The host of this server. Currently this ist just for info.
			host: "localhost"
			# **listenHost** *String* The express listen host
			listenHost: null
			# **basepath** *String* Path prefix for all routes.
			basepath: "/"
			# **title** *String* Express title
			title: "terraino-server"
			# **appname** *String* The app name. Used as `redis-session` namespace.
			appname: ""
			# **sessionttl** *Number* session timeout passed to connect-redis-sessions. Default is one month/31 days
			sessionttl: 1000 * 60 * 60 * 24 * 31
			# **templateCache** *Boolean* Use the express template cache
			templateCache: false

	###	
	## constructor 
	###
	constructor: ->
		super
		@express = express()
		
		@on "configured", @load
		@on "loaded", @start

		@configure()
		return

	###
	## configure
	
	`server.configure()`
	
	Configure the server
	
	@api private
	###
	configure: =>

		@debug "configue express"
		expressConf = Config.get( "express" )

		# set the express modules
		@express.set( "title", @config.title )
		@express.use( morgan( expressConf.logger ) )
		@express.use( compression() )
		@express.use( bodyParser.json() )
		@express.use( bodyParser.urlencoded( extended: true ) )

		# serve the static files
		@express.use( serveStatic( path.resolve( __dirname, "./static" ), maxAge: expressConf.staticCacheTime ) )
		
		
		
		# define the [nunjucks](http://mozilla.github.io/nunjucks/) environment
		nunjucksEnv = nunjucks.configure 'views',
			autoescape: true
			express: @express
			watch: true

		
		
		
		# attach the template renderer to express
		@express.set('views', path.resolve( __dirname, './views' ))
		@express.set('view engine', 'html')
		@express.set('view cache', @config.templateCache )

		
		@emit "configured"
		return

	###
	## load
	
	`server.load()`
	
	Loading and attachning the submodules to express
	
	@api private
	###
	load: =>
		@debug "load"

		# add ping endpoints
		@express.get "/ping", @ping
		@express.get "/ping.html", @ping

		# load rest modules 
		require( "./modules/rest/users" )
			.createRoutes( "/api/users", @express )
		
		
		# add GUI endpoints		
		@gui = require( "./modules/gui" )
		@gui.createRoutes( "/", @express )
		
		
		# init 404 route
		@express.all "*", @send404

		# add error handling
		@express.use ( err, req, res, next )=>
			@fatal "unkown-error", err
			res.send( err )
			return

		process.on "uncaughtException", ( err )=>
			@fatal "unkown-error", err
			return

		@emit "loaded"
		return

	###
	## ping
	
	`server.ping( req, res )`
	
	REspond the ping request with OK and the current version
	
	@param { Request } req Express Request 
	@param { Response } res Express Response 
	
	@api private
	###
	ping: ( req, res )=>
		res.send( "OK - #{Config.get( "version" )}" )
		return

	###
	## start
	
	`server.start()`
	
	Start listening to the defined port
	
	@param { String }  Desc 
	@param { Function }  Callback function 
	
	@api private
	###
	start: =>
		# we instantiate the app using express 2.x style in order to use socket.io
		server = http.createServer( @express )
		server.listen( @config.port, @config.listenHost )

		@info "start: listen to port #{@config.listenHost}:#{ @config.port }"
		return

	###
	## send404
	
	`server.send404( req, res )`
	
	Respond with a 404 page if the route is not defined
	
	@param { Request } req Express Request 
	@param { Response } res Express Response 
	
	@api private
	###
	send404: ( req, res )=>
		res.status( 404 )
		res.send( "Page not found!" )

		return

# create the instance and export it.
module.exports = new Server()

	