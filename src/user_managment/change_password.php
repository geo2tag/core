<!DOCTYPE HTML>
<html>	
<head>
	<title>Password changing</title>
	<meta charset="utf-8">
	<script src="geo2tag_requests.js"></script>
	<script src="spin.js"></script>

	<?php 
		include('db_interaction.php');
		$server_name=getServerAddress();
		echo "<script> SettingsStorage.SERVER_URL = '$server_name/service'</script>";
		
	?>

	<script>
	var target = document.getElementById('table');
	var spinner = new Spinner();

	function onSubmitButtonClicked(){

		var login = document.getElementById("login_edit").value;
		var old_password = document.getElementById("old_password_edit").value;
		var new_password = document.getElementById("new_password_edit").value;



	        if (!login || !old_password || !new_password){
			alert("Fields have incorrect values.");
			return;
		}

		spinner.spin(target);
		sendChangePasswordRequest(login, old_password, new_password, 
			onChangePasswordSuccess, onChangePasswordError);

	}

	function onChangePasswordSuccess(jsonResponse){
		spinner.stop();
		alert("Password changed successfuly!");
	}

	function onChangePasswordError(jsonResponse){
		spinner.stop();	
		alert("Something went wrong during requet processing.");
	}
		
	</script>

</head>

<body>
Geo2Tag LBS platform password changing page<br>
<table id="table" border>
        <tr>
                <td>
			Login<br>
			<input type="text" size="40" id="login_edit" title="Login" value="" ><br>
			Old Password<br>
			<input size="40" id="old_password_edit" title="Old Password" value="" type="password"><br>
			New Password<br>
			<input size="40" id="new_password_edit" title="New Password" value="" type="password"><br>
			<button onclick="onSubmitButtonClicked();">Submit</button><br>	
		</td>
	</tr>
</table>
</body>	
</html>

