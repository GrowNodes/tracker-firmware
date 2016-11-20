
var gulp  = require('gulp');
var uglify = require('gulp-uglify');
var cleanCSS = require('gulp-clean-css');
var gzip  = require('gulp-gzip');
var rename  = require('gulp-rename');
var inlinesource = require('gulp-inline-source');

// var watcher = gulp.watch('src/captive/*', ["prepare"]);
// watcher.on("change", function(event) {
//   console.log("File " + event.path + " was " + event.type + ", running tasks...");
// });

gulp.task("default", ["prepare"], function() {
  console.log("Build Finished");
});

gulp.task('prepare', ["uglify", "minify-css"], function () {
    return gulp.src('src/captive/index.html')
        .pipe(inlinesource())
        .pipe(gzip({gzipOptions:{level:9}, extension:'gz', append:true}))
        .pipe(rename("ui_bundle.gz"))
        .pipe(gulp.dest('data/homie'));
});

gulp.task('minify-css', function() {
  return gulp.src('src/captive/ui.css')
    .pipe(cleanCSS())
    .pipe(rename({ suffix: '.min' }))
    .pipe(gulp.dest('src/captive'));
});

gulp.task('uglify',  function() {
  return gulp.src('src/captive/ui.js')
        .pipe(uglify())
        .pipe(rename({ suffix: '.min' }))
        .pipe(gulp.dest('src/captive'));
});
