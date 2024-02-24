const express = require("express");
const app = express();
const { exec } = require("child_process");


// Enable json response, from: https://stackoverflow.com/questions/19696240/proper-way-to-return-json-using-node-or-express
app.use(express.json());
app.set('json spaces', 2);



// Handle HTTP requests:
const logRequest = function(req, res, next) {
        console.log(`Request: ${req.method} for ${req.path}`);
        next();
}

app.use(logRequest);

// import source files:
app.get("/hello", function(req, res) {
        res.json({ status: "ok", message: "hello test reqeust!" });
});

app.get("/federated_scheduling_test", function(req, res){
	exec('../NodeTimer/NodeTimer "../NodeTimer/TaskSetTest20240223.json" 6', (error, stdout, stderr) => {
		console.log(stdout);
		if (error) {
			console.log(error);
			console.log(stderr);
		}
		res.json({ status: "ok", message: stdout });
	});
});

// Server static files from the public dir
app.listen(3300, function() {
        console.log("Listeningg on port 3300...");
});
