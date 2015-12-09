class RestUsers extends require( "./restbase" )

	model: require( "../../model/users" )

	createRoutes: ( basepath, router )=>
		
		router.get "#{basepath}/:id", @get
		super
		return
		
	get: ( req, res )=>
		_id = req.params.id

		@model.get( _id, @_return( res ) )
		return

module.exports = new RestUsers()