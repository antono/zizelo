/*
Copyright (c) 2015 The Polymer Project Authors. All rights reserved.
This code may only be used under the BSD style license found at http://polymer.github.io/LICENSE.txt
The complete set of authors may be found at http://polymer.github.io/AUTHORS.txt
The complete set of contributors may be found at http://polymer.github.io/CONTRIBUTORS.txt
Code distributed by Google as part of the polymer project is also
subject to an additional IP rights grant found at http://polymer.github.io/PATENTS.txt
*/

'use strict';


//
// Include Gulp & Tools We'll Use
//
var gulp = require('gulp');
var plugins = require('gulp-load-plugins')();
var del = require('del');
var runSequence = require('run-sequence');
var path = require('path');

var DIST = 'dist';
var BASE_URL = '.';

//
// Task definitions
//
gulp.task('help', plugins.taskListing);

// PostCSS and friends
var BROWSERS = require('browserslist')('last 2 versions, > 5%');

var processors = [
  require('postcss-import')({
    path: ['app/styles']
  }),
  require('postcss-assets')({
    basePath: 'app/',
    loadPaths: ['app/fonts', 'app/images'],
    cachebuster: true
  }),
  require('postcss-position'),
  require('postcss-easings'),
  require('postcss-custom-properties'),
  require('postcss-calc'),
  require('postcss-discard-comments'),
  require('postcss-host'),
  require('autoprefixer-core')({ browsers: BROWSERS }),
  require('postcss-nested'),
  require('postcss-log-warnings')
];

// Compile and Automatically Prefix Stylesheets
gulp.task('styles', function () {
  return gulp.src(path.join('app/**/*.css'))
    .pipe(plugins.sourcemaps.init())
    .pipe(plugins.changed(DIST, {extension: '.css'}))
    .pipe(plugins.postcss(processors))
    .on('error', console.log.bind(console, 'PostCSS error'))
    .pipe(plugins.sourcemaps.write('.'))
    .pipe(gulp.dest(DIST))
    .pipe(plugins.size({title: 'styles'}))
    .pipe(plugins.connect.reload());
});

// Lint JavaScript
gulp.task('jshint', function () {
  return gulp.src([
      'app/scripts/**/*.js',
      'app/components/**/*.js',
      'app/components/**/*.html'
    ])
    .pipe(plugins.jshint())
    .pipe(plugins.jshint.reporter('jshint-stylish'))
    .pipe(plugins.connect.reload());
});

// Optimize Images
gulp.task('images', function () {
  return gulp.src('app/images/**/*')
    .pipe(plugins.cache(plugins.imagemin({
      progressive: true,
      interlaced: true
    })))
    .pipe(gulp.dest(DIST + '/images'))
    .pipe(plugins.size({title: 'images'}));
});

// Copy All Files At The Root Level (app)
gulp.task('copy', function () {
  return gulp.src(['app/*'], { dot: true })
    .pipe(gulp.dest(DIST))
    .pipe(plugins.size({title: 'copy'}));
});

gulp.task('symlink', function () {
  return gulp.src('./bower_components/')
    .pipe(plugins.symlink(DIST + '/bower_components'));
});

// Copy Web Fonts To Dist
gulp.task('fonts', function () {
  return gulp.src(['app/fonts/**'])
    .pipe(gulp.dest(DIST + '/fonts'))
    .pipe(plugins.size({title: 'fonts'}));
});

gulp.task('babel', function () {
  return gulp.src(['app/**/*.js'])
    .pipe(plugins.sourcemaps.init())
    .pipe(plugins.babel())
    .pipe(plugins.sourcemaps.write('.', {sourceRoot: '/app/' }))
    .pipe(plugins.debug())
    .pipe(gulp.dest(DIST));
});

gulp.task('jade', function () {
  return gulp.src('app/**/*.jade')
    .pipe(plugins.replace('BASE_URL', BASE_URL))
    .pipe(plugins.jade({ pretty: true }))
    .on('error', console.error.bind(console))
    .pipe(gulp.dest(DIST))
    .pipe(plugins.connect.reload());
});

// Vulcanize imports
gulp.task('vulcanize', function () {
  var DEST_DIR = DIST + '/components';

  return gulp.src(DIST + '/components/components.vulcanized.html')
    .pipe(plugins.vulcanize({
      dest: DEST_DIR,
      strip: true,
      inline: true
    }))
    .pipe(gulp.dest(DEST_DIR))
    .pipe(plugins.size({title: 'vulcanize'}));
});

// Clean Output Directory
gulp.task('clean', del.bind(null, [DIST]));

gulp.task('watch', ['default'], function () {
  gulp.watch(['app/**/*.jade'], ['jade']);
  gulp.watch(['app/styles/**/*.css'], ['styles']);
  gulp.watch(['app/components/**/*.css'], ['components']);
  gulp.watch(['app/{scripts,components}/**/*.js'], ['jshint', 'babel']);
});

gulp.task('server', ['watch'], function() {
  plugins.connect.server({
    root: [DIST, 'app'],
    port: 7777,
    livereload: true,
    // If you wnat bypas same origin policy when accessping API
    // middleware: function(connect, o) {
    //   return [(function() {
    //     var url = require('url');
    //     var proxy = require('proxy-middleware');
    //     var options = url.parse('http://localhost:3000/api');
    //     options.route = '/api';
    //     return proxy(options);
    //   })() ];
    // }
  });
});

// Build Production Files, the Default Task
gulp.task('default', ['clean'], function (cb) {
  runSequence(
    ['copy', 'symlink', 'styles'],
    'jade', 'babel',
    ['jshint', 'images', 'fonts'],
    'vulcanize',
    cb);
});

// Load tasks for web-component-tester
// Adds tasks for `gulp test:local` and `gulp test:remote`
try { require('web-component-tester').gulp.init(gulp); } catch (err) {}

// Load custom tasks from the `tasks` directory
try { require('require-dir')('tasks'); } catch (err) {}
