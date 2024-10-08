use uo;
use os;
use basic;

function isPromptSafe(prompt)
	var apiURL := "https://api.openai.com/v1/moderations";
	var method := "POST";
	var jsonData := PackJSON(struct{
		"input" := prompt
	});

	var apiResponse := HTTPRequest(
		apiURL,
		method,
		struct{
			data := jsonData,
			headers := struct{
				"Content-Type" := "application/json",
				"Authorization" := "Bearer {1}".format(GetEnvironmentVariable("OPENAI_KEY"))
			}
		}
	);

	var unpackedJSON := UnpackJSON(apiResponse);
	
	if (unpackedJSON["error"])
		return struct{ "error" := unpackedJSON["error"]["message"] };
	endif
	print(unpackedJSON["results"][1]["flagged"]);
	return unpackedJSON["results"][1]["flagged"];
endfunction

function OpenAICompletion(systemMessage, questions)
	var prompt := "";
    foreach question in questions
        if (question.user)
            prompt += question.user;
        endif
    endforeach
	prompt += systemMessage;

	if (isPromptSafe(prompt)) // terminando o processo, pois foi flaggado como Harmful/Unsafe
		return "";
	endif

	var formatedQuestions := array{ struct{
		"role" := "system",
		"content" := systemMessage
	} };

	foreach question in questions
		if (typeof(question) != "Struct")
			return struct{ "error" := "This must be an Struct" };
		endif
		if (!question.assistant && !question.user)
			return struct{ "error" := "Each question must be a struct containing the keys 'assistant' and 'user'" };
		endif

		if (question.assistant)
			formatedQuestions.append(struct{
				"role" := "assistant",
				"content" := question.assistant
			});
		endif

		if (question.user)
			formatedQuestions.append(struct{
				"role" := "user",
				"content" := question.user
			});
		endif
	endforeach

	var apiURL := "https://api.openai.com/v1/chat/completions";
	var method := "POST";
	var jsonData := PackJSON(struct{
		"model" := "gpt-4o-mini",
		"messages" := formatedQuestions
	});

	var apiResponse := HTTPRequest(
			apiURL,
			method,
			struct{
				data := jsonData,
				headers := struct{
				"Content-Type" := "application/json",
				"Authorization" := "Bearer {1}".format(GetEnvironmentVariable("OPENAI_KEY"))
				}
			}
		);
	var unpackedJSON := UnpackJSON(apiResponse);
	return unpackedJSON["choices"][1]["message"]["content"];
endfunction