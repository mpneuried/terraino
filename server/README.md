# terraino-server


## Install

Make sure you have installed the grunt client `npm install -g grunt-cli`.

```
npm install
grunt build
```



### Configuration

To change your local configuration add a file `config.json` to the root.
Within this file every key is optional

**Example:**

```
{
	"server": {
		"port": 5000
	}
}
```

#### Relevant configurations

* **server** *( `Object` )*: server configuration.
	* **port** *( `Number` [ default: `5000` ])*: The port the server will listen to.
	* **basepath** *( `String` [ default: `/` ])*: Path prefix for all routes.
	* **appname** *( `String` [ default: `` ])*: The app name. Used as `redis-session` namespace.
* **selfLink** *( `String` [ default: `http://localhost:5000/` ] )* The link to the running server. E.g. used to generate the passwordless token-link.
* **express** *( `Object` )*: Express configuration.
	* **logger** *( `String` [ default: `dev` ])*: Express logger config.
	* **staticCacheTime** *( `Number` [ default: `2678400000` ])*: Caching time for static content. Default is 31 days.


> More to come


### Run

To start the server just call

```
node server.js
```

### Development

It's recommened to use a tool like `node-dev` ( `npm install -g node-dev` ) for developement wich restarts the server on a file change.
Also you have 

**Start server**

```
node-dev server.js
```

Now you can use the grunt watcher to generate the project files on every source change.

```
grunt watch
```

Before you commit your code it's recommeded to call

```
grunt build-dev
```

This task will build all files and updates the code docs

## TODOS


## Release History
|Version|Date|Description|
|:--:|:--:|:--|
|0.0.1|2015-3-25|Initial commit|

> Initially Generated with [generator-mpnodeapp](https://github.com/mpneuried/generator-mpnodeapp)

## The MIT License (MIT)

Copyright © 2015 [mpneuried](https://github.com/mpneuried)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
