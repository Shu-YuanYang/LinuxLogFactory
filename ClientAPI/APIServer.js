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
	var task_set_ID = req.query.TaskSetID;
        var processor_count = req.query.ProcessorCount;
        console.log(task_set_ID);
	console.log(processor_count);
	if (task_set_ID.includes("/")){
		res.json({ status: "rejected", message: "task ID must not include special characters!" });
		return;
	}
	if (!parseInt(processor_count)){
		res.json({ status: "rejected", message: "processor count must be an integer!" });
		return;
	}

	var execution_str = `../NodeTimer/NodeTimer "../NodeTimer/${task_set_ID}.json" ${processor_count}`;

	exec('../NodeTimer/NodeTimer "../NodeTimer/TaskSetTest20240223.json" 6', (error, stdout, stderr) => {
		console.log(stdout);
		if (error) {
			console.log(error);
			console.log(stderr);
		}
		res.json({ status: "ok", message: stdout });
	});
});

/*
app.post("/query/query_str", function(req, res){
                var queryID = req.query.queryID;
                var query_body = req.body;
                var url_address = "https://6dvsvj47eujpghwa6xxxksnzi40oqxzl.lambda-url.us-east-2.on.aws/visualdata/query_str?queryID=" + queryID;
                //console.log(JSON.stringify(query_body));
                fetch(url_address, {
                        method: 'post',
                        headers: { 
                                "Content-Type": "application/json"
                        },
                        body: JSON.stringify(query_body)
                })
                .then(CloudSharpResult => CloudSharpResult.json())
                .then(result => { res.json(result); })
                .catch(err => { res.sendStatus(501); });
});
*/



// Server static files from the public dir
app.listen(3300, function() {
        console.log("Listeningg on port 3300...");
});
