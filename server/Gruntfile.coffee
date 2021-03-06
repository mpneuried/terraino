path = require( "path" )

request = require( "request" )
_ = require( "lodash" )

try 
	_gconfig = require( "./config.json" )?.grunt
	if not _gconfig?
		console.log( "INFO: No grunt config in `config.json` found. So use default.\n" )
		_gconfig = {}
catch _e
	if _e.code is "MODULE_NOT_FOUND"
		console.log( "INFO: No `config.json` found. So use default.\n" ) 
	_gconfig = {}

_.defaults( _gconfig, {
	"gettext_path": "/usr/local/opt/gettext/bin/"
})

languageCodes = [ 'de', 'en' ]

module.exports = (grunt) ->
	# Project configuration.
	grunt.initConfig
		pkg: grunt.file.readJSON('package.json')
		gconfig: _gconfig
		#deploy: deploy
		regarde:
			serverjs:
				files: ["_src/**/*.coffee"]
				tasks: [ "coffee:serverchanged" ]
			
			frontendjs:
				files: ["_src_static/js/**/*.coffee"]
				tasks: [ "build_staticjs" ]
			frontendvendorjs:
				files: ["_src_static/js/vendor/**/*.js"]
				tasks: [ "build_staticjs" ]
			frontendcss:
				files: ["_src_static/css/**/*.styl"]
				tasks: [ "stylus" ]
			
			frontendtemplates:
				files: ["_src_static/jst/**/*.html", "views/componentes/**/*.html"]
				tasks: [ "build_frontend" ]
			
			static:
				files: ["_src_static/static/**/*.*"]
				tasks: [ "build_staticfiles" ]
			
			#i18nserver:
			#	files: ["_locale/**/*.po"]
			#	tasks: [ "build_i18n_server" ]
		
		coffee:
			serverchanged:
				expand: true
				cwd: '_src'
				src:	[ '<% print( _.first( ((typeof grunt !== "undefined" && grunt !== null ? (_ref = grunt.regarde) != null ? _ref.changed : void 0 : void 0) || ["_src/nothing"]) ).slice( "_src/".length ) ) %>' ]
				# template to cut off `_src/` and throw on error on non-regrade call
				# CF: `_.first( grunt?.regarde?.changed or [ "_src/nothing" ] ).slice( "_src/".length )
				dest: ''
				ext: '.js'
			
			frontendchanged:
				expand: true
				cwd: '_src_static/js'
				src:	[ '<% print( _.first( ((typeof grunt !== "undefined" && grunt !== null ? (_ref = grunt.regarde) != null ? _ref.changed : void 0 : void 0) || ["_src_static/js/nothing"]) ).slice( "_src_static/js/".length ) ) %>' ]
				# template to cut off `_src_static/js/` and throw on error on non-regrade call
				# CF: `_.first( grunt?.regarde?.changed or [ "_src_static/js/nothing" ] ).slice( "_src_static/js/".length )
				dest: 'static_tmp/js'
				ext: '.js'
			
			backend_base:
				expand: true
				cwd: '_src',
				src: ["**/*.coffee"]
				dest: ''
				ext: '.js'
			
			frontend_base:
				expand: true
				cwd: '_src_static/js',
				src: ["**/*.coffee"]
				dest: 'static_tmp/js'
				ext: '.js'
			


		clean:
			server:
				src: [ "lib", "modules", "model", "models", "*.js", "release", "test" ]
			
			frontend: 
				src: [ "static", "static_tmp" ]
			mimified: 
				src: [ "static/js/*.js", "!static/js/main.js" ]
			statictmp: 
				src: [ "static_tmp" ]
			
			

		
		stylus:
			standard:
				options:
					"include css": true
				files:
					"static/css/style.css": ["_src_static/css/style.styl"]
					"static/css/login.css": ["_src_static/css/login.styl"]

		
		nunjucks:
			main:
				baseDir: "_src_static/jst/",
				src: ["_src_static/jst/**/*.html", "views/components/*.html"],
				dest: 'static_tmp/js/jst.js'
				options: 
					name: ( filename )->
						return filename.replace( /^views\//i, "" )

		
		browserify: 
			main: 
				src: [ 'static_tmp/js/main.js' ]
				dest: 'static/js/main.js'				
				options:
					alias: [
						"nunjucks/browser/nunjucks-slim.js:nunjucks",
						"./static_tmp/js/jst.js:jst"
					]
			login: 
				src: [ 'static_tmp/js/login.js' ]
				dest: 'static/js/login.js'

		copy:
			static:
				expand: true
				cwd: '_src_static/static',
				src: [ "**" ]
				dest: "static/"
			bootstrap_fonts:
				expand: true
				cwd: 'node_modules/bootstrap/dist/fonts',
				src: [ "**" ]
				dest: "static/fonts/"

		uglify:
			options:
				banner: '/*!<%= pkg.name %> - v<%= pkg.version %>\n*/\n'
			staticjs:
				files:
					"static/js/main.js": [ "static/js/main.js" ]
		
		cssmin:
			options:
				banner: '/*! <%= pkg.name %> - v<%= pkg.version %>*/\n'
			staticcss:
				files:
					"static/css/external.css": [ "_src_static/css/*.css", "node_modules/bootstrap/dist/css/bootstrap.css" ]
		
		compress:
			main:
				options: 
					archive: "release/<%= pkg.name %>_deploy_<%= pkg.version.replace( '.', '_' ) %>.zip"
				files: [
						{ src: [ "package.json", "server.js", "modules/**", "lib/**", "static/**", "views/**", "_src_static/css/**/*.styl" ], dest: "./" }
				]

		

		
		

	# Load npm modules
	grunt.loadNpmTasks "grunt-regarde"
	grunt.loadNpmTasks "grunt-contrib-coffee"
	grunt.loadNpmTasks "grunt-contrib-stylus"
	grunt.loadNpmTasks "grunt-contrib-uglify"
	grunt.loadNpmTasks "grunt-contrib-cssmin"
	grunt.loadNpmTasks "grunt-contrib-copy"
	grunt.loadNpmTasks "grunt-contrib-compress"
	grunt.loadNpmTasks "grunt-contrib-concat"
	grunt.loadNpmTasks "grunt-contrib-clean"
	
	grunt.loadNpmTasks "grunt-nunjucks"
	grunt.loadNpmTasks "grunt-browserify"
	
	



	# just a hack until this issue has been fixed: https://github.com/yeoman/grunt-regarde/issues/3
	grunt.option('force', not grunt.option('force'))
	
	# ALIAS TASKS
	grunt.registerTask "watch", "regarde"
	grunt.registerTask "default", "build"
	
	
	grunt.registerTask "clear", [ "clean:server", "clean:frontend"  ]

	# build the project
	
	grunt.registerTask "build", [ "clean:frontend", "build_server", "build_frontend"  ]
	grunt.registerTask "build-dev", [ "build"  ]

	grunt.registerTask "build_server", [ "coffee:backend_base" ]

	
	grunt.registerTask "build_frontend", [ "build_staticjs", "build_vendorcss", "stylus", "build_staticfiles" ]
	grunt.registerTask "build_staticjs", [ "clean:statictmp", "coffee:frontend_base", "nunjucks:main", "browserify:main", "clean:mimified" ]
	grunt.registerTask "build_vendorcss", [ "cssmin:staticcss" ]
	grunt.registerTask "build_staticfiles", [ "copy:static", "copy:bootstrap_fonts" ]
	

	grunt.registerTask "release", [ "build", "uglify:staticjs", "compress" ]