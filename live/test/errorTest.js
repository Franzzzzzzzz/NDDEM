const fs = require('fs');
const path = require('path');

module.exports = {
  '@tags': ['errorTest'],

  'Check for errors within 5 seconds after page load': function (browser) {
    const htmlFilesFolder = 'live/deploy'; // Replace with the path to your HTML files folder

    const htmlFiles = fs.readdirSync(htmlFilesFolder).filter(file => path.extname(file) === '.html');

    htmlFiles.forEach(file => {
      const filePath = path.join(htmlFilesFolder, file);
      browser
        .url(browser.launch_url + filePath) // Load the local HTML file
        .waitForElementVisible('canvas', 5000) // Wait for the page to load for 5 seconds
        .execute(function () {
          return window.errors; // Assuming the page sets the 'errors' variable when an error occurs
        }, [], function (result) {
          const errors = result.value;
          if (errors && errors.length > 0) {
            console.error(`Errors detected in ${file}:`, errors);
          } else {
            console.log(`No errors found in ${file}.`);
          }
        });
    });

    browser.end(); // End the test
  }
};