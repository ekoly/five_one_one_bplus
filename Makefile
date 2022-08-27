all:
	python setup.py sdist bdist_wheel

clean:
	rm -rf dist/ build/ *.egg-info .fuse_hidden*

install:
	python setup.py install

reinstall:
	$(MAKE) clean
	pip uninstall -y five-one-one-bplus
	$(MAKE) install

test: reinstall
	python -m pytest
