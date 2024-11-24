dic = {
    "path tracing": {
        "label": "path tracing",
        "description": "path tracing integrator",
        "parameters": {
            "max_depth": {
                "type": "Int",
                "args": {
                    "default": 8,
                    "max": 50,
                    "min": 1
                }
            },
            "min_depth": {
                "type": "Int",
                "args": {
                    "default": 3,
                    "max": 10,
                    "min": 0
                }
            },
            "rr_threshold": {
                "type": "Int",
                "args": {
                    "default": 3,
                    "max": 10,
                    "min": 0
                }
            }
        }
    }
}