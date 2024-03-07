const express = require("express");
const cors = require("cors");
const { exec } = require("child_process");
const path = require("path");
const JSZip = require('jszip');
const fs = require('fs');

const app = express();





function zip_files_unsafe(zip_id, file_folder_path, filename_list, on_finish) {
	const zip = new JSZip();
	try {
		for (let i = 0; i < filename_list.length; ++i) {
			const file_full_path = `${file_folder_path}/${filename_list[i]}`;
			if (!fs.existsSync(file_full_path)) throw new Error('file does not exist: ' + file_full_path);
			const fileData = fs.readFileSync(file_full_path);
			zip.file(filename_list[i], fileData);
		}

		zip.generateNodeStream({ type: 'nodebuffer', streamFiles: true })
			.pipe(fs.createWriteStream(`${file_folder_path}/${zip_id}.zip`))
			.on('finish', function () { console.log(`${zip_id}.zip written.`); on_finish(zip_id); });
	} catch (err) {
		console.error(err);
	}
}


function zip_files(zip_id, file_folder_path, filename_list, on_finish) {
	const zip_full_path = `${file_folder_path}/${zip_id}.zip`;
	if (fs.existsSync(zip_full_path)) {
		//console.log("unlink")
		fs.unlink(zip_full_path, (err) => {
			if (err) { console.error(err); return; }
			zip_files_unsafe(zip_id, file_folder_path, filename_list, on_finish);
		});
	}
	else {
		//console.log("zip");
		zip_files_unsafe(zip_id, file_folder_path, filename_list, on_finish);
	}
}





// Enable CORS:
app.use(cors());


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



const SCHEDULING_PROGRAM_FOLDER = "../NodeTimer";
const SCHEDULING_TASK_FILES_FOLDER = `${SCHEDULING_PROGRAM_FOLDER}/task_files`; 
function validate_scheduling_test_query(task_set_ID, processor_count) {
	if (task_set_ID.includes("/")){
                //res.json({ status: "rejected", message: "task ID must not include special characters!" });
                return { status: "rejected", message: "task ID must not include special characters!" };
        }
        if (!parseInt(processor_count)){
                //res.json({ status: "rejected", message: "processor count must be an integer!" });
                return { status: "rejected", message: "processor count must be an integer!" };
        }
	return { status: "accepted", message: "input validated" };
}

app.post("/federated_scheduling_test", function(req, res){
	var task_set_ID = req.query.TaskSetID;
        var processor_count = req.query.ProcessorCount;
        //console.log(task_set_ID);
	//console.log(processor_count);
	//console.log(req.body);
	validation_status = validate_scheduling_test_query(task_set_ID, processor_count);
	if (validation_status.status != "accepted") res.json(validation_status);
	/*if (task_set_ID.includes("/")){
		res.json({ status: "rejected", message: "task ID must not include special characters!" });
		return;
	}
	if (!parseInt(processor_count)){
		res.json({ status: "rejected", message: "processor count must be an integer!" });
		return;
	}*/

	var execution_str = `${SCHEDULING_PROGRAM_FOLDER}/NodeTimer "${SCHEDULING_TASK_FILES_FOLDER}/${task_set_ID}.json" ${processor_count}`;

	exec(execution_str, (error, stdout, stderr) => {
		//console.log(stdout);
		if (error) {
			console.log(error);
			console.log(stderr);
		}
		res.json({ status: "ok", message: stdout });
	});
});


app.post("/federated_scheduling_download", function(req, res){
	var task_set_ID = req.query.TaskSetID;
        var processor_count = req.query.ProcessorCount;
        //console.log(task_set_ID);
        //console.log(processor_count);
        //console.log(req.body);
        validation_status = validate_scheduling_test_query(task_set_ID, processor_count);
        if (validation_status.status != "accepted") res.json(validation_status);

	var filename_list = [];
	if (req.body.download_task_set) { 
		filename_list.push(`${task_set_ID}.json`);
	}
	if (req.body.download_schedule) {
		filename_list.push(`${task_set_ID}.csv`);
	}
	zip_files(task_set_ID, SCHEDULING_TASK_FILES_FOLDER, filename_list, function(zip_id) {
                var p = path.join(__dirname, `${SCHEDULING_TASK_FILES_FOLDER}/${zip_id}.zip`);
                //console.log(p);
                res.sendFile(p);
	});
});




// Server static files from the public dir
app.listen(3300, function() {
        console.log("Listeningg on port 3300...");
});
