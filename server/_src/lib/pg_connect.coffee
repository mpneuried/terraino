# # pg Base
# ### extends [NPM:MPBasic](https://cdn.rawgit.com/mpneuried/mpbaisc/master/_docs/index.coffee.html)
#
# ### Exports: *Class*
# 
# Generic class to handle postgres data

# **npm modules**
pg = require( "pg" )

# **internal modules**
# [Config](../lib/config.coffee.html)
Config = require( "../lib/config" )

class PG extends require( "mpbasic" )( Config )

	# ## defaults
	defaults: =>
		return @extend true, super,
			# **url** *String* The host url to the postgres server
			url: "localhost"
			# **port** *Number* The port the postgres server running on
			port: 5432
			# **username** *String* The user name to connect to the server
			username: null
			# **password** *String* The password to connect to the server
			password: null
			# **database** *String* The databsename to connect to
			database: null

	constructor: ->
		@connected = false
		@exec = @_waitUntil( @_exec, "connected" )

		@on( "connect", @connect )
		@getter( "conString", @getConnectionString, false )
		super
		return

	connect: =>
		if @connected
			return

		console.log "connect to pg"
		pg.connect @conString, ( err, client, release )=>
			if err
				@error "CONNECTION ERROR", err
				@emit "error", err
				return

			@client = client

			@once( "release", release )
			@connected = true
			@emit( "connected" )
			return

		return

	release: =>
		@emit "release"
		return

	exec: ( query, args..., cb )=>
		console.log "pg conn?", @connected
		if not @connected
			@emit "connect"
			return 

		console.log "exec pq"
		@client.query query, args, ( err, result )=>
			if err
				cb( err )
				return
			@_processResult( result, cb )
			return
		return

	_processResult: ( result, cb )=>
		cb( null, result.rows )
		return

	getConnectionString: =>
		if process.env.DATABASE_URL?
			return process.env.DATABASE_URL

		_s = "postgres://"
		if @config.username?.length and @config.password?.length
			_s += "#{@config.username}:#{@config.password}@"
		_s += "#{@config.url}"
		_s += "/#{@config.database}"

		return _s


module.exports = new PG()