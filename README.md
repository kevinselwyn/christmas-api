#christmas-api

API that returns whether or not it is Christmas Day

##Installation

```bash
make && sudo make install
```

##Usage

```
christmas-api (<port>)
              (-t|--template <template.json>)
              (-v|--verbose)
              (-h|--help)
```

| Flag          | Description                                   | Default       |
| ------------- | --------------------------------------------- | ------------- |
| <port>        | Port to broadcast API on                      | 8000          |
| -t,--template | JSON template, use `%s` for true/false result | template.json |
| -v,--verbose  | Displays responses from clients               | false         |
| -h,--help     | Provides help                                 | N/A           |

##Example

On December 25:

```json
{
	"christmas": true
}
```

On any other day:

```json
{
	"christmas": false
}
```

##Acknowledgements

Credit for inspiration should be given to [isitchristmas.com](https://isitchristmas.com/) and other similar sites. May this API spawn many copycats.