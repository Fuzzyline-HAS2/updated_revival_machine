$env:PYTHONDONTWRITEBYTECODE = "1"
python -m pytest tests -v -p no:cacheprovider
